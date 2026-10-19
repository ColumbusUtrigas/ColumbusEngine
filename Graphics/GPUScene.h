#pragma once

#include "Core/StableSparseArray.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "Graphics/Particles/ParticleEmitterCPU.h"
#include "Mesh.h"
#include "Camera.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Light.h"
#include "Graphics/Material.h"
#include "Graphics/Vulkan/DeviceVulkan.h"
#include "Profiling/Profiling.h"
#include <Core/Core.h>
#include <span>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>

namespace Columbus
{

	// design idea:
	// the GPU scene is immutable during the rendering and is created as a "proxy" from
	// the CPU representation.
	// after update/upload of a GPU scene, it can be used during rendering in render graph
	// Similar concept as Unreal's RenderProxy, but simple

	///////////////////////////////////////////////////////////////////////////////////////////////
	// everything for an instance of a mesh

	struct GPUSceneMesh;
	struct GPUSceneMeshInstance;

	using HStableMaterialId       = TStableSparseArray<Material>::Handle;
	using HStableMeshId           = TStableSparseArray<GPUSceneMesh>::Handle;
	using HStableMeshInstanceId   = TStableSparseArray<GPUSceneMeshInstance>::Handle;

	// Render-side state shared by all primitive proxies belonging to one mesh instance.
	struct GPUSceneMeshInstance
	{
		std::vector<HStableMeshId> Primitives;
		std::vector<Matrix> SkinMatrices;
		std::vector<float> MorphWeights;

		u64 SkinMatricesAddress = 0;
		u32 SkinMatrixCount = 0;

		u64 MorphWeightsAddress = 0;
		u32 MorphWeightCount = 0;
	};

	// Per-primitive dynamic deformation resources owned by a GPU scene primitive proxy.
	struct GPUMeshDeform
	{
		Buffer* Vertices     = nullptr;
		Buffer* PrevVertices = nullptr;
		Buffer* Normals      = nullptr;
		Buffer* Tangents     = nullptr;
		AccelerationStructure* BLAS = nullptr;

		u32 VertexCount = 0;
		u32 FramesSinceFullBLASBuild = 0;
	};

	// Primitive render proxy consumed by mesh upload, drawing, and TLAS construction.
	struct GPUSceneMesh
	{
		GPUMeshResource* MeshResource = nullptr;
		GPUMeshDeform* Deform = nullptr;
		HStableMeshInstanceId Instance{};

		Matrix Transform;
		Matrix PrevTransform = Matrix(1);
		HStableMaterialId MaterialId{};
		int LightmapId = -1;
		bool MotionHistoryValid = false;
		bool UploadedThisFrame  = false;
	};

	// Spans passed from game/editor code when creating or updating a mesh instance.
	struct GPUSceneMeshInstanceState
	{
		Matrix Transform = Matrix(1);
		std::span<const HStableMaterialId> Materials;
		std::span<const Matrix> SkinMatrices;
		std::span<const float> MorphWeights;
	};

	// Shader-visible packed primitive record uploaded into GPUScene.MeshesBuffer.
	struct GPUSceneMeshCompact
	{
		Matrix Transform;
		Matrix PrevTransform;
		Matrix NormalMatrix;

		u64 VertexBufferAddress;
		u64 IndexBufferAddress;
		u64 Uv1BufferAddress;
		u64 Uv2BufferAddress;
		u64 NormalsBufferAddress;
		u64 TangentsBufferAddress;

		u32 VertexCount;
		u32 IndexCount;

		int MaterialId;
		int LightmapId;

		u64 PrevVertexBufferAddress;
		u32 MotionHistoryValid;
		u32 _pad0;
	};

	static_assert(sizeof(GPUSceneMeshCompact) == 272, "GPUSceneMeshCompact must match shader layout");

	struct GPUCamera
	{
		Vector4 Position;
		Vector4 Direction;
		Vector4 Up;
		Vector4 Side;
		// TODO: focal length, focal distance, aperture, sensor, etc...

		GPUCamera() {}
		GPUCamera(const Camera& FromCamera)
		{
			Position = Vector4(FromCamera.Pos, 0);
			Direction = Vector4(FromCamera.Direction(), 0);
			Up = Vector4(FromCamera.Up(), 0);
			Side = Vector4(FromCamera.Right(), 0);
		}

		bool operator==(const GPUCamera&) const = default;
		bool operator!=(const GPUCamera&) const = default;
	};

	struct GPULightCompact
	{
		Vector4 Position;
		Vector4 Direction; // xyz: toward light for punctual/directional, emitting normal for planar area; w: emitter roll.
		Vector4 Color;
		LightType Type;
		float Range;
		float SourceRadius;
		float VolumetricIntensity;
		ELightFlags Flags; // 64
		Vector2 SizeOrSpotAngles; // 76

		u32   ShadowResolution  = 1024;
		u32   FirstShadowView   = 0;
		u32   ShadowViewCount   = 0;
		u32   RTShadowMaskIndex = 0;
		float ShadowDepthBias   = 0.0001f; // Reverse-Z receiver comparison bias.
		float ShadowNormalBias  = 0.01f;   // World units.
		float ShadowSlopeBias   = 2.0f;    // Raster slope-scaled depth bias.
		float ShadowFilterRadius = 1.0f;    // Fixed 3x3 PCF sample spacing in texels (0 = one hardware-PCF sample).
		float ShadowDistance     = 100.0f;
		u32   ShadowCascadeCount = 4;
		float ShadowSplitLambda  = 0.6f;
		float ShadowCascadeBlend = 0.1f;
		float ShadowConeAngle    = 150.0f;
	};

	static_assert(sizeof(GPULightCompact) == 128, "GPULightCompact must match shader layout");

	struct GPULight : GPULightCompact
	{
		// all GPULightCompact fields plus some state

		float ShadowNearPlane = 0.02f;

		// CPU-only local-light interactions. Keys contain primitive sparse index + generation.
		std::unordered_map<u64, u32> ShadowCasters; // Primitive key -> shadow face mask.
		u64 ShadowRevision   = 1;
		std::array<u32, 6> ShadowCasterCounts{};

		bool NeedsRTShadowDenoiser() const;
	};

	GPULightCompact CreateCompact(const GPULight& Light);

	struct GPUVolumetricFogSettings
	{
		Vector4 AlbedoDensity; // rgb albedo, a density
		Vector4 Params0; // x height falloff, y height offset, z max distance, w anisotropy
		Vector4 Params1; // x noise scale, y noise amount, z shadow strength, w unused
		iVector4 Control; // x enabled, y froxel pixel size, z froxel slices, w integration steps
		iVector4 Control2; // x max lights
	};

	struct GPUDecal
	{
		Matrix Model;
		Matrix ModelInverse;
		Texture2* Texture;

		// TODO: remove that, handle internally
		VkDescriptorSet _DescriptorSets[MaxFramesInFlight]{NULL};
	};

	struct GPUMaterialCompact
	{
		Vector4 AlbedoFactor;
		Vector4 EmissiveFactor;

		int AlbedoId;
		int NormalId;
		int OrmID;
		int EmissiveId;

		float Roughness;
		float Metallic;

		int Flags;
		int ShadingMode;
		float AlphaCutoff;
		int _pad[3];
	};

	static constexpr int GPUMATERIAL_FLAG_NORMAL_RG = 1 << 0;

	struct GPUViewCamera
	{
		Matrix ViewMatrix;
		Matrix ProjectionMatrix;
		Matrix ViewProjectionMatrix; // 192

		Matrix InverseViewMatrix;
		Matrix InverseProjectionMatrix;
		Matrix InverseViewProjectionMatrix; // 384

		Vector4 CameraPosition;
		Vector4 CameraForward;
		Vector4 CameraUp;
		Vector4 CameraRight; // 448

		Vector2 Jittering; // 456

		int _pad[14]; // 512
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Baked lighting

	// layout must match Shaders/IrradianceVolume/IrradianceVolumeCommon.hlsli
	struct GPUIrradianceProbe
	{
		alignas(16) Vector4 PositionState;
		alignas(16) Vector4 Irradiance[6];
		Vector2 Distance[64];
	};

	struct GPUIrradianceVolume
	{
		int ProbeCountX = 0;
		int ProbeCountY = 0;
		int ProbeCountZ = 0;
		int ProbeStride = 0;

		float Intensity = 1.0f;
		float NormalBias = 0.2f;
		float BlendDistance = 0.0f;
		float Priority = 0.0f;

		Vector3 Position;
		Vector3 Extent;

		Buffer* Probes = nullptr; // owned by GPUScene

		iVector3 GetProbesCount() const { return iVector3(ProbeCountX, ProbeCountY, ProbeCountZ); }
		int GetTotalProbes() const { return ProbeCountX * ProbeCountY * ProbeCountZ; }
	};

	struct GPUReflectionCapture
	{
		int Resolution = 0;
		int MipCount = 0;

		float Intensity = 1.0f;
		float BlendDistance = 0.0f;
		float Priority = 0.0f;

		Vector3 Position;
		Vector3 Extent;

		// prefiltered radiance cubemap, owned by GPUScene.
		// RGBA32F while a bake is in flight, BC6H once baked data is applied.
		Texture2* Radiance = nullptr;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////

	// per-particle data
	struct GPUParticleCompact
	{
		Vector4 Position_Rotation; // xyz=pos, w=rot
		Vector4 Size_Frame;        // xyz=scale, w=frame
		Vector4 Colour;            // xyzw=colour
	};

	// particles render proxy
	struct GPUSceneParticles
	{
		HParticleEmitterInstanceCPU* ParticleInstance; // data coming into GPUScene
		Buffer* DataBuffer = nullptr; // GPU packed particles data
	};

	// to be uploaded to the GPU
	struct GPUSceneCompact
	{
		// view description
		GPUViewCamera CameraCur;
		GPUViewCamera CameraPrev;

		iVector2 RenderSize;
		iVector2 OutputSize;

		// scene description
		Vector4 SunDirection;
		Vector4 SkySHR; // SH2 Red
		Vector4 SkySHG; // SH2 Green
		Vector4 SkySHB; // SH2 Blue

		u32 MeshesCount;
		u32 MaterialsCount;
		u32 TexturesCount;
		u32 LightsCount;
		u32 DecalsCount;

		u32 _pad[3]; // 1136

		HSkySettings Sky;
		GPUVolumetricFogSettings VolumetricFog;
	};

	using HStableLightId     = TStableSparseArray<GPULight>::Handle;
	using HStableTextureId   = TStableSparseArray<Texture2*>::Handle;
	using HStableDecalId     = TStableSparseArray<GPUDecal>::Handle;
	using HStableParticlesId = TStableSparseArray<GPUSceneParticles>::Handle;
	using HStableIrradianceVolumeId  = TStableSparseArray<GPUIrradianceVolume>::Handle;
	using HStableReflectionCaptureId = TStableSparseArray<GPUReflectionCapture>::Handle;

	struct GPUScene
	{
		static constexpr int MaxMeshes    = 8192;
		static constexpr int MaxMaterials = 8192;
		static constexpr int MaxGPULights = 8192;
		static constexpr int MaxDecals    = 8192;
		static constexpr int MaxSkinMatrices = 65536;
		static constexpr int MaxMorphWeights = 65536;

		AccelerationStructure* TLAS = nullptr;

		TStableSparseArray<GPUSceneMeshInstance> MeshInstances;
		TStableSparseArray<GPUSceneMesh> Meshes;
		TStableSparseArray<Material>     Materials;
		TStableSparseArray<Texture2*>    Textures;
		TStableSparseArray<GPULight>     Lights;
		TStableSparseArray<GPUDecal>     Decals;
		TStableSparseArray<GPUSceneParticles> Particles;

		// static GI
		TStableSparseArray<GPUIrradianceVolume>  IrradianceVolumes;
		TStableSparseArray<GPUReflectionCapture> ReflectionCaptures;

		SPtr<DeviceVulkan> Device;

		GPUCamera MainCamera;
		bool Dirty = false;
		size_t ShadowMaterialHash = 0;

		// ClearWorld requests a reset; the scene's deferred view consumes it on its next render.
		bool ResetHistory = false;

		// copied over from World
		HSkySettings Sky;
		HVolumetricFogSettings VolumetricFog;

		Vector4 SunDirection = Vector4(1,1,1,0);

		// GPUScene also owns GPU-specific resources
		Buffer* SceneBuffer = nullptr;
		Buffer* LightsBuffer = nullptr;
		Buffer* MeshesBuffer = nullptr;
		Buffer* MaterialsBuffer = nullptr;
		Buffer* DecalsBuffers = nullptr;
		Buffer* SkinMatricesBuffer = nullptr;
		Buffer* MorphWeightsBuffer = nullptr;

		// Linearly Transformed Cosines (LTC)
		Texture2* LTC_1 = nullptr;
		Texture2* LTC_2 = nullptr;

		// split sum approximation stuff
		Texture2* BRDF_LUT = nullptr;

		// Shared by shader-side diagnostics; independent of the editor UI font.
		Texture2* DebugFontAtlas = nullptr;

	public:
		void Update();
		void SavePreviousMeshTransforms();
		Box GetMeshBounds(const GPUSceneMesh& Mesh);
		void UpdateShadowCaster(HStableMeshId Id, bool Removed = false);
		void RebuildShadowCasters(GPULight& Light);

		bool IsMeshInstanceValid(HStableMeshInstanceId Id) const;
		HStableMeshInstanceId AddMeshInstance(Mesh2& MeshAsset, const GPUSceneMeshInstanceState& State);
		void RemoveMeshInstance(HStableMeshInstanceId Id);
		void UpdateMeshInstance(HStableMeshInstanceId Id, const GPUSceneMeshInstanceState& State);

		HStableLightId AddLight(const GPULight& Light);
		void UpdateLight(HStableLightId Id, const GPULight& Light);
		void RemoveLight(HStableLightId Id);

		HStableParticlesId AddParticleSystem(HParticleEmitterInstanceCPU* Emitter);
		void DeleteParticleSystem(HStableParticlesId Id);

		// Baked lighting. Parameter fields on GPU* entries are written directly by the owning
		// things; GPU resources (probe buffers, radiance cubemaps) are created/destroyed only
		// through these functions so destruction and memory tracking stay in one place.
		HStableIrradianceVolumeId AddIrradianceVolume();
		void RemoveIrradianceVolume(HStableIrradianceVolumeId Id);
		void SetIrradianceProbes(GPUIrradianceVolume& Volume, const void* Data, u64 Size);
		void ReleaseIrradianceProbes(GPUIrradianceVolume& Volume, bool bDeferred = true);

		HStableReflectionCaptureId AddReflectionCapture();
		void RemoveReflectionCapture(HStableReflectionCaptureId Id);
		// (re)creates the RGBA32F full-mip-chain bake target if missing or mismatched, updates MipCount
		Texture2* EnsureReflectionBakeCubemap(GPUReflectionCapture& Capture);
		// replaces the radiance cubemap with a texture created from a (BC6H) cube image
		bool SetReflectionRadiance(GPUReflectionCapture& Capture, const Image& CubeImage);
		void ReleaseReflectionRadiance(GPUReflectionCapture& Capture, bool bDeferred = true);

		GPUSceneCompact CreateCompact(const RenderView& View) const;

		static GPUScene* CreateGPUScene(SPtr<DeviceVulkan> Device);
		static void DestroyGPUScene(GPUScene* Scene, SPtr<DeviceVulkan> Device);
	};

	// it's split up in two functions because populating/updating of a GPUScene might be done in different ways
	// and can be split up in different stages
	// void UpdateGPUScene(SPtr<DeviceVulkan> Device, CPUScene* cpuScene, GPUScene* gpuScene);
	// void UploadGPUScene();

	// GPUArray is a helper type used to pack both
	// Count and Data in one contiguous buffer
	template <typename T>
	struct GPUArray
	{
		uint32_t Count;
		static constexpr int Padding = 12;

		static size_t Bytesize(uint32_t InCount)
		{
			return sizeof(GPUArray::Count) + Padding + sizeof(T) * InCount;
		}

		size_t Bytesize()
		{
			return Bytesize(Count);
		}

		T& operator[](size_t Index)
		{
			return *(T*)(((char*)this) + sizeof(Count) + Padding + sizeof(T) * Index);
		}

		static SPtr<GPUArray<T>> Allocate(uint32_t InCount)
		{
			size_t Size = Bytesize(InCount);
			GPUArray<T>* Result = static_cast<GPUArray<T>*>(::operator new(Size));
			Result->Count = InCount;
			// Result->Data = Result + offsetof(GPUArray, Data);
			return SPtr<GPUArray<T>>(Result);
		}
	};

}
