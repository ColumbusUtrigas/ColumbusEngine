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
#include "IrradianceVolume.h"
#include <Core/Core.h>
#include <vector>

namespace Columbus
{

	// design idea:
	// the GPU scene is immutable during the rendering and is created as a "proxy" from
	// the CPU representation.
	// after update/upload of a GPU scene, it can be used during rendering in render graph
	// Similar concept as Unreal's RenderProxy, but simple

	// instance of a mesh
	struct GPUSceneMesh
	{
		GPUMeshResource* MeshResource;

		Matrix Transform;
		int MaterialId = -1;
		int LightmapId = -1;
	};

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

	// to be uploaded into the GPU
	struct GPUSceneMeshCompact
	{
		Matrix Transform;

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

		// 128
	};

	struct GPULight
	{
		Vector4 Position;
		Vector4 Direction;
		Vector4 Color;
		LightType Type;
		float Range;
		float SourceRadius;
		ELightFlags Flags; // 64
		Vector2 SizeOrSpotAngles; // 72

		u32 _pad[14]; // 128
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

		int _pad[2];
	};

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
	};

	using HStableLightId     = TStableSparseArray<GPULight>::Handle;
	using HStableTextureId   = TStableSparseArray<Texture2*>::Handle;
	using HStableMaterialId  = TStableSparseArray<Material>::Handle;
	using HStableMeshId      = TStableSparseArray<GPUSceneMesh>::Handle;
	using HStableDecalId     = TStableSparseArray<GPUDecal>::Handle;
	using HStableParticlesId = TStableSparseArray<GPUSceneParticles>::Handle;


	struct GPUScene
	{
		static constexpr int MaxMeshes    = 8192;
		static constexpr int MaxMaterials = 8192;
		static constexpr int MaxGPULights = 8192;
		static constexpr int MaxDecals    = 8192;

		AccelerationStructure* TLAS = nullptr;

		TStableSparseArray<GPUSceneMesh> Meshes;
		TStableSparseArray<Material>     Materials;
		TStableSparseArray<Texture2*>    Textures;
		TStableSparseArray<GPULight>     Lights;
		TStableSparseArray<GPUDecal>     Decals;
		TStableSparseArray<GPUSceneParticles> Particles;

		std::vector<IrradianceVolume> IrradianceVolumes;

		SPtr<DeviceVulkan> Device;

		GPUCamera MainCamera;
		bool Dirty = false;

		// copied over from World
		HSkySettings Sky;

		Vector4 SunDirection = Vector4(1,1,1,0);

		// GPUScene also owns GPU-specific resources
		Buffer* SceneBuffer = nullptr;
		Buffer* LightsBuffer = nullptr;
		Buffer* MeshesBuffer = nullptr;
		Buffer* MaterialsBuffer = nullptr;
		Buffer* DecalsBuffers = nullptr;

		// Linearly Transformed Cosines (LTC)
		Texture2* LTC_1 = nullptr;
		Texture2* LTC_2 = nullptr;

	public:
		void Update();

		HStableParticlesId AddParticleSystem(HParticleEmitterInstanceCPU* Emitter);
		void DeleteParticleSystem(HStableParticlesId Id);

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
