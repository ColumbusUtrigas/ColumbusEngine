#include "GPUScene.h"

#include "BRDFLUT.h"
#include "LTC.h"
#include "Graphics/Core/ShaderDebugText.h"

#include <filesystem>
#include <stdio.h>

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Animation/deform", "SceneMemory", MemoryCounter_SceneAnimationDeform);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Irradiance probes", "SceneMemory", MemoryCounter_SceneIrradianceProbes);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Reflection captures", "SceneMemory", MemoryCounter_SceneReflectionCaptures);

namespace Columbus
{
	static void CreateGPUSceneBuffers(SPtr<DeviceVulkan> Device, u64 Size, const char* Name, Buffer*& Buf)
	{
		BufferDesc Desc;
		Desc.Size = Size;
		Desc.BindFlags = BufferType::UAV;
		Buf = Device->CreateBuffer(Desc, nullptr);
		Device->SetDebugName(Buf, Name);
	}

	static void DestroyGPUSceneBuffers(SPtr<DeviceVulkan> Device, Buffer* Buf)
	{
		Device->DestroyBuffer(Buf);
	}

	static Buffer* CreateSceneMeshBuffer(SPtr<DeviceVulkan> Device, size_t Size, bool bUsedInAS, const void* Data)
	{
		BufferDesc Desc;
		Desc.BindFlags = BufferType::UAV;
		Desc.Size = Size;
		Desc.UsedInAccelerationStructure = bUsedInAS;
		Buffer* Result = Device->CreateBuffer(Desc, Data);
		AddProfilingMemory(MemoryCounter_SceneMeshes, Result->GetSize());
		AddProfilingMemory(MemoryCounter_SceneAnimationDeform, Result->GetSize());
		return Result;
	}

	static void DestroySceneMeshBuffer(SPtr<DeviceVulkan> Device, Buffer* Buffer)
	{
		if (Buffer)
		{
			RemoveProfilingMemory(MemoryCounter_SceneMeshes, Buffer->GetSize());
			RemoveProfilingMemory(MemoryCounter_SceneAnimationDeform, Buffer->GetSize());
		}
		Device->DestroyBufferDeferred(Buffer);
	}

	static void DestroySceneMeshDeform(SPtr<DeviceVulkan> Device, GPUMeshDeform* Deform)
	{
		if (Deform == nullptr)
			return;

		DestroySceneMeshBuffer(Device, Deform->Vertices);
		DestroySceneMeshBuffer(Device, Deform->PrevVertices);
		DestroySceneMeshBuffer(Device, Deform->Normals);
		DestroySceneMeshBuffer(Device, Deform->Tangents);

		if (Deform->BLAS)
		{
			RemoveProfilingMemory(MemoryCounter_SceneBLAS, Deform->BLAS->GetSize());
			RemoveProfilingMemory(MemoryCounter_SceneAnimationDeform, Deform->BLAS->GetSize());
		}
		Device->DestroyAccelerationStructure(Deform->BLAS);

		*Deform = {};
	}

	static GPUMeshDeform* CreateSceneMeshDeform(SPtr<DeviceVulkan> Device, const Mesh2* MeshAsset, const MeshPrimitive& Primitive, const std::string& DebugName)
	{
		if (Device == nullptr || MeshAsset == nullptr)
			return nullptr;

		const CPUMeshResource& CPU = Primitive.CPU;
		const GPUMeshResource& GPU = Primitive.GPU;
		const bool bHasSkinningInput =
			CPU.BoneIndices.size() == CPU.Vertices.size() &&
			CPU.BoneWeights.size() == CPU.Vertices.size() &&
			GPU.BoneIndices != nullptr &&
			GPU.BoneWeights != nullptr;
		const bool bHasMorphInput =
			!CPU.MorphTargets.empty() &&
			GPU.MorphDeltas != nullptr &&
			GPU.MorphTargetCount > 0;

		if (CPU.Vertices.empty() ||
			CPU.Normals.size() != CPU.Vertices.size() ||
			CPU.Tangents.size() != CPU.Vertices.size() ||
			GPU.Indices == nullptr ||
			(!bHasSkinningInput && !bHasMorphInput))
		{
			return nullptr;
		}

		GPUMeshDeform* Deform = new GPUMeshDeform();
		Deform->VertexCount = (u32)CPU.Vertices.size();
		Deform->Vertices = CreateSceneMeshBuffer(Device, sizeof(Vector3) * CPU.Vertices.size(), true, CPU.Vertices.data());
		Deform->PrevVertices = CreateSceneMeshBuffer(Device, sizeof(Vector3) * CPU.Vertices.size(), false, CPU.Vertices.data());
		Deform->Normals = CreateSceneMeshBuffer(Device, sizeof(Vector3) * CPU.Normals.size(), false, CPU.Normals.data());
		Deform->Tangents = CreateSceneMeshBuffer(Device, sizeof(Vector4) * CPU.Tangents.size(), false, CPU.Tangents.data());

		if (!DebugName.empty())
		{
			Device->SetDebugName(Deform->Vertices, (DebugName + " Deform Vertices").c_str());
			Device->SetDebugName(Deform->PrevVertices, (DebugName + " Deform PrevVertices").c_str());
			Device->SetDebugName(Deform->Normals, (DebugName + " Deform Normals").c_str());
			Device->SetDebugName(Deform->Tangents, (DebugName + " Deform Tangents").c_str());
		}

		if (MeshAsset->ImportSettings.BuildBLAS)
		{
			AccelerationStructureDesc BlasDesc;
			BlasDesc.Type = AccelerationStructureType::BLAS;
			BlasDesc.Vertices = Deform->Vertices;
			BlasDesc.Indices = GPU.Indices;
			BlasDesc.VerticesCount = (int)GPU.VertexCount;
			BlasDesc.IndicesCount = (int)GPU.IndicesCount;
			BlasDesc.AllowUpdate = true;
			BlasDesc.PreferFastBuild = true;
			Deform->BLAS = Device->CreateAccelerationStructure(BlasDesc);

			if (Deform->BLAS)
			{
				if (!DebugName.empty())
					Device->SetDebugName(Deform->BLAS, (DebugName + " Deform BLAS").c_str());
				AddProfilingMemory(MemoryCounter_SceneBLAS, Deform->BLAS->GetSize());
				AddProfilingMemory(MemoryCounter_SceneAnimationDeform, Deform->BLAS->GetSize());
			}
		}

		return Deform;
	}

	static GPUViewCamera CreateCameraCompact(const Camera& Camera)
	{
		return GPUViewCamera
		{
			.ViewMatrix           = Camera.GetViewMatrix(),
			.ProjectionMatrix     = Camera.GetProjectionMatrix(),
			.ViewProjectionMatrix = Camera.GetViewProjection(),

			.InverseViewMatrix           = Camera.GetViewMatrix().GetInverted(),
			.InverseProjectionMatrix     = Camera.GetProjectionMatrix().GetInverted(),
			.InverseViewProjectionMatrix = Camera.GetViewProjection().GetInverted(),

			.CameraPosition = Vector4(Camera.Pos, 0),
			.CameraForward  = Vector4(Camera.Direction(), 0),
			.CameraUp       = Vector4(Camera.Up(), 0),
			.CameraRight    = Vector4(Camera.Right(), 0),
			.Jittering      = Camera.GetJittering(),
		};
	}

	void RTShadowDenoiserHistory::Destroy(SPtr<DeviceVulkan> Device)
	{
		Device->DestroyTextureDeferred(Shadow);
		Device->DestroyTextureDeferred(Moments);

		Shadow = nullptr;
		Moments = nullptr;
		Valid = false;
	}

	bool GPULight::NeedsRTShadowDenoiser() const
	{
		if ((Flags & ELightFlags::Shadow) == (ELightFlags)0)
		{
			return false;
		}

		// if very dim it doesn't affect lighting too much
		if (Color.XYZ().Length() < 0.001f)
			return false;

		return SourceRadius > 0.0001f ||
			((Type == LightType::Rectangle || Type == LightType::Disc) && (SizeOrSpotAngles.X > 0.0001f || SizeOrSpotAngles.Y > 0.0001f));
	}

	GPULightCompact CreateCompact(const GPULight& Light)
	{
		return GPULightCompact{
			.Position            = Light.Position,
			.Direction           = Light.Direction,
			.Color               = Light.Color,
			.Type                = Light.Type,
			.Range               = Light.Range,
			.SourceRadius        = Light.SourceRadius,
			.VolumetricIntensity = Light.VolumetricIntensity,
			.Flags               = Light.Flags,
			.SizeOrSpotAngles    = Light.SizeOrSpotAngles,
			.ShadowResolution    = Light.ShadowResolution,
			.FirstShadowView     = Light.FirstShadowView,
			.ShadowViewCount     = Light.ShadowViewCount,
			.RTShadowMaskIndex   = Light.RTShadowMaskIndex,
			.ShadowDepthBias     = Light.ShadowDepthBias,
			.ShadowNormalBias    = Light.ShadowNormalBias,
			.ShadowSlopeBias     = Light.ShadowSlopeBias,
			.ShadowFilterRadius  = Light.ShadowFilterRadius,
			.ShadowDistance      = Light.ShadowDistance,
			.ShadowCascadeCount  = Light.ShadowCascadeCount,
			.ShadowSplitLambda   = Light.ShadowSplitLambda,
			.ShadowCascadeBlend  = Light.ShadowCascadeBlend,
			.ShadowConeAngle     = Light.ShadowConeAngle,
		};
	}

	void GPUScene::Update()
	{
		for (int i = 0; i < (int)Lights.Size(); i++)
		{
			const GPULight& SceneLight = Lights.Data()[i];

			// select first directional light as sun direction
			if (SceneLight.Type == LightType::Directional)
			{
				SunDirection = Vector4(SceneLight.Direction.XYZ().Normalized(), 0);
			}
		}
	}

	bool GPUScene::IsMeshInstanceValid(HStableMeshInstanceId Id) const
	{
		return MeshInstances.IsValid(Id);
	}

	HStableMeshInstanceId GPUScene::AddMeshInstance(Mesh2& MeshAsset, const GPUSceneMeshInstanceState& State)
	{
		assert(State.Materials.size() == MeshAsset.Primitives.size());
		assert(State.MorphWeights.size() == MeshAsset.Morphs.size());

		GPUSceneMeshInstance Instance;
		Instance.Primitives.reserve(MeshAsset.Primitives.size());
		Instance.SkinMatrices.assign(State.SkinMatrices.begin(), State.SkinMatrices.end());
		Instance.MorphWeights.assign(State.MorphWeights.begin(), State.MorphWeights.end());

		const HStableMeshInstanceId InstanceId = MeshInstances.Add(Instance);
		GPUSceneMeshInstance* StoredInstance = MeshInstances.Get(InstanceId);
		assert(StoredInstance != nullptr);

		for (size_t PrimitiveIndex = 0; PrimitiveIndex < MeshAsset.Primitives.size(); PrimitiveIndex++)
		{
			MeshPrimitive& Primitive = MeshAsset.Primitives[PrimitiveIndex];
			assert(Primitive.CPU.MorphTargets.size() == MeshAsset.Morphs.size());

			GPUSceneMesh Mesh;
			Mesh.MeshResource = &Primitive.GPU;
			Mesh.Instance = InstanceId;
			Mesh.Transform = State.Transform;
			Mesh.PrevTransform = State.Transform;
			Mesh.MaterialId = State.Materials[PrimitiveIndex];

			std::string DebugName;
			if (!MeshAsset.SourcePath.empty())
				DebugName = std::filesystem::path(MeshAsset.SourcePath).filename().string();
			if (!Primitive.Name.empty())
				DebugName += DebugName.empty() ? Primitive.Name : (" " + Primitive.Name);

			Mesh.Deform = CreateSceneMeshDeform(Device, &MeshAsset, Primitive, DebugName);
			StoredInstance->Primitives.push_back(Meshes.Add(Mesh));
		}

		return InstanceId;
	}

	void GPUScene::RemoveMeshInstance(HStableMeshInstanceId Id)
	{
		GPUSceneMeshInstance* Instance = MeshInstances.Get(Id);
		assert(Instance != nullptr);

		for (HStableMeshId PrimitiveId : Instance->Primitives)
		{
			GPUSceneMesh* Mesh = Meshes.Get(PrimitiveId);
			assert(Mesh != nullptr);
			DestroySceneMeshDeform(Device, Mesh->Deform);
			delete Mesh->Deform;
			Mesh->Deform = nullptr;
			Meshes.Remove(PrimitiveId);
		}

		MeshInstances.Remove(Id);
	}

	void GPUScene::UpdateMeshInstance(HStableMeshInstanceId Id, const GPUSceneMeshInstanceState& State)
	{
		GPUSceneMeshInstance* Instance = MeshInstances.Get(Id);
		assert(Instance != nullptr);
		assert(State.Materials.size() == Instance->Primitives.size());
		assert(State.SkinMatrices.size() == Instance->SkinMatrices.size());
		assert(State.MorphWeights.size() == Instance->MorphWeights.size());

		Instance->SkinMatrices.assign(State.SkinMatrices.begin(), State.SkinMatrices.end());
		Instance->MorphWeights.assign(State.MorphWeights.begin(), State.MorphWeights.end());

		for (size_t PrimitiveIndex = 0; PrimitiveIndex < Instance->Primitives.size(); PrimitiveIndex++)
		{
			GPUSceneMesh* Mesh = Meshes.Get(Instance->Primitives[PrimitiveIndex]);
			assert(Mesh != nullptr);
			Mesh->Transform = State.Transform;
			Mesh->MaterialId = State.Materials[PrimitiveIndex];
		}
	}

	void GPUScene::SavePreviousMeshTransforms()
	{
		for (u32 i = 0; i < (u32)Meshes.Size(); i++)
		{
			Meshes[i].PrevTransform      = Meshes[i].Transform;
			Meshes[i].MotionHistoryValid = Meshes[i].UploadedThisFrame;
			Meshes[i].UploadedThisFrame  = false;
		}
	}

	HStableLightId GPUScene::AddLight(const GPULight& Light)
	{
		return Lights.Add(Light);
	}

	void GPUScene::UpdateLight(HStableLightId Id, const GPULight& Light)
	{
		GPULight* Existing = Lights.Get(Id);
		if (Existing == nullptr)
		{
			return;
		}

		*Existing = Light;
	}

	void GPUScene::RemoveLight(HStableLightId Id)
	{
		Lights.Remove(Id);
	}

	HStableParticlesId GPUScene::AddParticleSystem(HParticleEmitterInstanceCPU* Emitter)
	{
		GPUSceneParticles Resource;
		Resource.ParticleInstance = Emitter;

		BufferDesc Desc;
		Desc.BindFlags = BufferType::UAV;
		Desc.Size = Emitter->Settings->MaxParticles * sizeof(GPUParticleCompact);

		Resource.DataBuffer = Device->CreateBuffer(Desc, nullptr);
		Device->SetDebugName(Resource.DataBuffer, "GPUScene.ParticleBuffer");

		return Particles.Add(Resource);
	}

	void GPUScene::DeleteParticleSystem(HStableParticlesId Id)
	{
		GPUSceneParticles* Resource = Particles.Get(Id);
		if (Resource && Resource->DataBuffer)
		{
			Device->DestroyBufferDeferred(Resource->DataBuffer);
		}

		Particles.Remove(Id);
	}

	HStableIrradianceVolumeId GPUScene::AddIrradianceVolume()
	{
		return IrradianceVolumes.Add(GPUIrradianceVolume{});
	}

	void GPUScene::RemoveIrradianceVolume(HStableIrradianceVolumeId Id)
	{
		if (GPUIrradianceVolume* Volume = IrradianceVolumes.Get(Id))
		{
			ReleaseIrradianceProbes(*Volume, true);
		}

		IrradianceVolumes.Remove(Id);
	}

	void GPUScene::SetIrradianceProbes(GPUIrradianceVolume& Volume, const void* Data, u64 Size)
	{
		ReleaseIrradianceProbes(Volume, true);

		if (Data == nullptr || Size == 0)
			return;

		BufferDesc Desc;
		Desc.BindFlags = BufferType::UAV;
		Desc.Size = Size;
		Volume.Probes = Device->CreateBuffer(Desc, Data);
		Volume.ProbeStride = sizeof(GPUIrradianceProbe);
		Device->SetDebugName(Volume.Probes, "GPUScene.IrradianceProbes");
		AddProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume.Probes->GetSize());
	}

	void GPUScene::ReleaseIrradianceProbes(GPUIrradianceVolume& Volume, bool bDeferred)
	{
		if (Volume.Probes == nullptr)
			return;

		RemoveProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume.Probes->GetSize());
		if (bDeferred)
			Device->DestroyBufferDeferred(Volume.Probes);
		else
			Device->DestroyBuffer(Volume.Probes);
		Volume.Probes = nullptr;
	}

	HStableReflectionCaptureId GPUScene::AddReflectionCapture()
	{
		return ReflectionCaptures.Add(GPUReflectionCapture{});
	}

	void GPUScene::RemoveReflectionCapture(HStableReflectionCaptureId Id)
	{
		if (GPUReflectionCapture* Capture = ReflectionCaptures.Get(Id))
		{
			ReleaseReflectionRadiance(*Capture, true);
		}

		ReflectionCaptures.Remove(Id);
	}

	Texture2* GPUScene::EnsureReflectionBakeCubemap(GPUReflectionCapture& Capture)
	{
		if (Capture.Resolution <= 0)
			return nullptr;

		const int MipCount = (int)ImageUtils::ImageFullMipCount((u32)Capture.Resolution, (u32)Capture.Resolution, 1);

		TextureDesc2 Desc;
		Desc.Type = TextureType::TextureCube;
		Desc.Usage = TextureUsage::StorageSampled;
		Desc.Width = (u32)Capture.Resolution;
		Desc.Height = (u32)Capture.Resolution;
		Desc.Mips = (u32)MipCount;
		Desc.Format = TextureFormat::RGBA32F;
		Desc.AddressU = TextureAddressMode::ClampToEdge;
		Desc.AddressV = TextureAddressMode::ClampToEdge;
		Desc.AddressW = TextureAddressMode::ClampToEdge;
		Desc.MinFilter = TextureFilter2::Linear;
		Desc.MagFilter = TextureFilter2::Linear;
		Desc.MipFilter = TextureFilter2::Linear;
		Desc.MaxLOD = (float)(MipCount - 1);

		if (Capture.Radiance != nullptr && Capture.Radiance->GetDesc() == TextureUtils::NormalizeTextureDesc(Desc))
		{
			Capture.MipCount = MipCount;
			return Capture.Radiance;
		}

		ReleaseReflectionRadiance(Capture, true);

		Capture.Radiance = Device->CreateTexture(Desc);
		Capture.MipCount = MipCount;
		if (Capture.Radiance != nullptr)
		{
			Device->SetDebugName(Capture.Radiance, "GPUScene.ReflectionCaptureRadiance");
			AddProfilingMemory(MemoryCounter_SceneReflectionCaptures, Capture.Radiance->GetSize());
		}

		return Capture.Radiance;
	}

	bool GPUScene::SetReflectionRadiance(GPUReflectionCapture& Capture, const Image& CubeImage)
	{
		TextureDesc2 Desc;
		Desc.Usage = TextureUsage::Sampled;
		TextureUtils::SetTextureDescImageFields(Desc, CubeImage);
		Desc.AddressU = TextureAddressMode::ClampToEdge;
		Desc.AddressV = TextureAddressMode::ClampToEdge;
		Desc.AddressW = TextureAddressMode::ClampToEdge;
		Desc.MinFilter = TextureFilter2::Linear;
		Desc.MagFilter = TextureFilter2::Linear;
		Desc.MipFilter = TextureFilter2::Linear;
		Desc.MaxLOD = (float)Math::Max(0u, Desc.Mips - 1);

		Texture2* NewRadiance = Device->CreateTexture(CubeImage, Desc);
		if (NewRadiance == nullptr)
			return false;

		ReleaseReflectionRadiance(Capture, true);

		Capture.Radiance = NewRadiance;
		Capture.MipCount = (int)Math::Max(1u, (u32)CubeImage.MipMaps);
		Device->SetDebugName(Capture.Radiance, "GPUScene.ReflectionCaptureRadiance");
		AddProfilingMemory(MemoryCounter_SceneReflectionCaptures, Capture.Radiance->GetSize());
		return true;
	}

	void GPUScene::ReleaseReflectionRadiance(GPUReflectionCapture& Capture, bool bDeferred)
	{
		if (Capture.Radiance == nullptr)
			return;

		RemoveProfilingMemory(MemoryCounter_SceneReflectionCaptures, Capture.Radiance->GetSize());
		if (bDeferred)
			Device->DestroyTextureDeferred(Capture.Radiance);
		else
			Device->DestroyTexture(Capture.Radiance);
		Capture.Radiance = nullptr;
	}

	GPUSceneCompact GPUScene::CreateCompact(const RenderView& View) const
	{
		Vector4 SHR(1, 0, 0, 0);
		Vector4 SHG(1, 0, 0, 0);
		Vector4 SHB(1, 0, 0, 0);

		HSkySettings SkyConverted = Sky;
		HVolumetricFogSettings FogConverted = VolumetricFog;
		// convert units
		{
			SkyConverted.SunAngularRadius = Math::Radians(SkyConverted.SunAngularRadius);
			SkyConverted.SunDiscSoftness *= 0.001f;

			SkyConverted.PlanetRadiusKm     *= 1000;
			SkyConverted.AtmosphereHeightKm *= 1000;
			SkyConverted.RayleightHeightKm  *= 1000;
			SkyConverted.MieHeight          *= 1000;
			SkyConverted.OzonePeak          *= 1000;
			SkyConverted.OzoneFalloff       *= 1000;

			FogConverted.Density = Math::Max(0.0f, FogConverted.Density);
			FogConverted.HeightFalloff = Math::Max(0.0f, FogConverted.HeightFalloff);
			FogConverted.MaxDistance = Math::Max(1.0f, FogConverted.MaxDistance);
			FogConverted.Anisotropy = Math::Clamp(FogConverted.Anisotropy, -0.95f, 0.95f);
			FogConverted.NoiseScale = Math::Max(0.0001f, FogConverted.NoiseScale);
			FogConverted.NoiseAmount = Math::Clamp(FogConverted.NoiseAmount, 0.0f, 1.0f);
			FogConverted.ShadowStrength = Math::Clamp(FogConverted.ShadowStrength, 0.0f, 2.0f);
			FogConverted.FroxelPixelSize = Math::Clamp(FogConverted.FroxelPixelSize, 4, 64);
			FogConverted.FroxelSlices = Math::Clamp(FogConverted.FroxelSlices, 8, 256);
			FogConverted.IntegrationSteps = Math::Clamp(FogConverted.IntegrationSteps, 4, 256);
			FogConverted.MaxLights = Math::Clamp(FogConverted.MaxLights, 1, 256);
		}

		GPUVolumetricFogSettings FogGPU{
			.AlbedoDensity = Vector4(FogConverted.Albedo.X, FogConverted.Albedo.Y, FogConverted.Albedo.Z, FogConverted.Density),
			.Params0 = Vector4(FogConverted.HeightFalloff, FogConverted.HeightOffset, FogConverted.MaxDistance, FogConverted.Anisotropy),
			.Params1 = Vector4(FogConverted.NoiseScale, FogConverted.NoiseAmount, FogConverted.ShadowStrength, 0.0f),
			.Control = iVector4(FogConverted.EnableVolumetricFog ? 1 : 0, FogConverted.FroxelPixelSize, FogConverted.FroxelSlices, FogConverted.IntegrationSteps),
			.Control2 = iVector4(FogConverted.MaxLights, 0, 0, 0),
		};

		return GPUSceneCompact
		{
			.CameraCur  = CreateCameraCompact(View.CameraCur),
			.CameraPrev = CreateCameraCompact(View.CameraPrev),

			.RenderSize = View.RenderSize,
			.OutputSize = View.OutputSize,

			.SunDirection = SunDirection,
			.SkySHR       = SHR,
			.SkySHG       = SHG,
			.SkySHB       = SHB,

			.MeshesCount    = (u32)Meshes.Size(),
			.MaterialsCount = (u32)Materials.Size(),
			.TexturesCount  = (u32)Textures.Size(),
			.LightsCount    = (u32)Lights.Size(),
			.DecalsCount    = (u32)Decals.Size(),

			.Sky = SkyConverted,
			.VolumetricFog = FogGPU,
		};
	}

	GPUScene* GPUScene::CreateGPUScene(SPtr<DeviceVulkan> Device)
	{
		GPUScene* Scene = new GPUScene();
		Scene->Device = Device;

		CreateGPUSceneBuffers(Device, sizeof(GPUSceneCompact), "GPUScene.Scene", Scene->SceneBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxGPULights * sizeof(GPULightCompact), "GPUScene.Lights", Scene->LightsBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMeshes * sizeof(GPUSceneMeshCompact), "GPUScene.Meshes", Scene->MeshesBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMaterials * sizeof(GPUMaterialCompact), "GPUScene.Materials", Scene->MaterialsBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxDecals * sizeof(GPUDecal), "GPUScene.Decals", Scene->DecalsBuffers);
		CreateGPUSceneBuffers(Device, GPUScene::MaxSkinMatrices * sizeof(Matrix), "GPUScene.SkinMatrices", Scene->SkinMatricesBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMorphWeights * sizeof(float), "GPUScene.MorphWeights", Scene->MorphWeightsBuffer);
		AddProfilingMemory(MemoryCounter_SceneAnimationDeform, Scene->SkinMatricesBuffer->GetSize());
		AddProfilingMemory(MemoryCounter_SceneAnimationDeform, Scene->MorphWeightsBuffer->GetSize());

		// populate LTC look up tables
		{
			TextureDesc2 LTCDesc;
			LTCDesc.Width = 64;
			LTCDesc.Height = 64;
			LTCDesc.Format = TextureFormat::RGBA32F;
			LTCDesc.MinFilter = TextureFilter2::Nearest;
			LTCDesc.MagFilter = TextureFilter2::Linear;
			LTCDesc.AddressU = TextureAddressMode::ClampToEdge;
			LTCDesc.AddressV = TextureAddressMode::ClampToEdge;

			Scene->LTC_1 = Device->CreateTexture(LTCDesc);
			Scene->LTC_2 = Device->CreateTexture(LTCDesc);

			Device->UploadTextureMipData(Scene->LTC_1, 0, 0, LTC::LTC_1);
			Device->UploadTextureMipData(Scene->LTC_2, 0, 0, LTC::LTC_2);
		}

		// populate split-sum environment BRDF look up table
		{
			TextureDesc2 BRDFDesc;
			BRDFDesc.Width = BRDFLUT::BRDF_LUT_SIZE;
			BRDFDesc.Height = BRDFLUT::BRDF_LUT_SIZE;
			BRDFDesc.Format = TextureFormat::RG16F;
			BRDFDesc.MinFilter = TextureFilter2::Linear;
			BRDFDesc.MagFilter = TextureFilter2::Linear;
			BRDFDesc.AddressU = TextureAddressMode::ClampToEdge;
			BRDFDesc.AddressV = TextureAddressMode::ClampToEdge;

			Scene->BRDF_LUT = Device->CreateTexture(BRDFDesc);
			Device->UploadTextureMipData(Scene->BRDF_LUT, 0, 0, BRDFLUT::BRDF_LUT_RG16F);
		}
		
		Scene->DebugFontAtlas = ShaderDebugText::CreateFontAtlas(Device);

		// allocate empty TLAS
		{
			AccelerationStructureDesc TlasDesc{};
			TlasDesc.Type = AccelerationStructureType::TLAS;
			TlasDesc.Instances = {};

			for (int i = 0; i < MaxMeshes; i++)
			{
				TlasDesc.Instances.push_back({ Matrix(1), nullptr });
			}

			Scene->TLAS = Device->CreateAccelerationStructure(TlasDesc);
		}

		return Scene;
	}

	void GPUScene::DestroyGPUScene(GPUScene* Scene, SPtr<DeviceVulkan> Device)
	{
		for (GPUSceneMesh& Mesh : Scene->Meshes)
		{
			DestroySceneMeshDeform(Device, Mesh.Deform);
			delete Mesh.Deform;
			Mesh.Deform = nullptr;
		}

		for (const auto& ParticleEntry : Scene->Particles)
		{
			if (ParticleEntry.DataBuffer)
			{
				Device->DestroyBuffer(ParticleEntry.DataBuffer);
			}
		}

		for (GPUIrradianceVolume& Volume : Scene->IrradianceVolumes)
		{
			Scene->ReleaseIrradianceProbes(Volume, false);
		}

		for (GPUReflectionCapture& Capture : Scene->ReflectionCaptures)
		{
			Scene->ReleaseReflectionRadiance(Capture, false);
		}

		DestroyGPUSceneBuffers(Device, Scene->SceneBuffer);
		DestroyGPUSceneBuffers(Device, Scene->LightsBuffer);
		DestroyGPUSceneBuffers(Device, Scene->MeshesBuffer);
		DestroyGPUSceneBuffers(Device, Scene->MaterialsBuffer);
		DestroyGPUSceneBuffers(Device, Scene->DecalsBuffers);
		RemoveProfilingMemory(MemoryCounter_SceneAnimationDeform, Scene->SkinMatricesBuffer->GetSize());
		RemoveProfilingMemory(MemoryCounter_SceneAnimationDeform, Scene->MorphWeightsBuffer->GetSize());
		DestroyGPUSceneBuffers(Device, Scene->SkinMatricesBuffer);
		DestroyGPUSceneBuffers(Device, Scene->MorphWeightsBuffer);

		Device->DestroyTexture(Scene->LTC_1);
		Device->DestroyTexture(Scene->LTC_2);
		Device->DestroyTexture(Scene->BRDF_LUT);
		ShaderDebugText::DestroyFontAtlas(Device, Scene->DebugFontAtlas);

		Device->DestroyAccelerationStructure(Scene->TLAS);

		Scene->Device.reset();
		delete Scene;
	}
}
