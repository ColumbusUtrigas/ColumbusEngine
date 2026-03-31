#include "GPUScene.h"

#include "LTC.h"

#include <stdio.h>

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
		CreateGPUSceneBuffers(Device, GPUScene::MaxGPULights * sizeof(GPULight), "GPUScene.Lights", Scene->LightsBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMeshes * sizeof(GPUSceneMeshCompact), "GPUScene.Meshes", Scene->MeshesBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxMaterials * sizeof(GPUMaterialCompact), "GPUScene.Materials", Scene->MaterialsBuffer);
		CreateGPUSceneBuffers(Device, GPUScene::MaxDecals * sizeof(GPUDecal), "GPUScene.Decals", Scene->DecalsBuffers);

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
		for (const auto& ParticleEntry : Scene->Particles)
		{
			if (ParticleEntry.DataBuffer)
			{
				Device->DestroyBuffer(ParticleEntry.DataBuffer);
			}
		}

		DestroyGPUSceneBuffers(Device, Scene->SceneBuffer);
		DestroyGPUSceneBuffers(Device, Scene->LightsBuffer);
		DestroyGPUSceneBuffers(Device, Scene->MeshesBuffer);
		DestroyGPUSceneBuffers(Device, Scene->MaterialsBuffer);
		DestroyGPUSceneBuffers(Device, Scene->DecalsBuffers);

		Device->DestroyTexture(Scene->LTC_1);
		Device->DestroyTexture(Scene->LTC_2);

		Device->DestroyAccelerationStructure(Scene->TLAS);

		Scene->Device.reset();
		delete Scene;
	}
}
