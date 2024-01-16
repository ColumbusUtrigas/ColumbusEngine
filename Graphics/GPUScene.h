#pragma once

#include "Graphics/Core/GraphicsCore.h"
#include "Camera.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Light.h"
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

	struct GPUSceneMesh
	{
		AccelerationStructure* BLAS;
		Matrix Transform;
		Buffer* Vertices;
		Buffer* Indices;
		Buffer* UV1;
		Buffer* UV2 = nullptr;
		Buffer* Normals;
		u32 VertexCount;
		u32 IndicesCount;

		int TextureId = -1; // TODO: proper materials
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

		u32 VertexCount;
		u32 IndexCount;

		// int MaterialId; // TODO:
		int TextureId;
		int LightmapId;
		
		int _pad[2]; // 128
	};

	struct GPULight
	{
		Vector4 Position;
		Vector4 Direction;
		Vector4 Color;
		LightType Type;
		float Range;
		float SourceRadius;
		u32 Padding[1];
	};

	// TODO: plug it into GPUScene
	struct GPUDecal
	{
		Matrix Model;
		Matrix ModelInverse;
		Texture2* Texture;

		VkDescriptorSet _DescriptorSets[MaxFramesInFlight]{NULL};
	};

	// to be uploaded to the GPU
	struct GPUSceneCompact
	{
		// TODO:
		// GPUCamera Camera; // 64
		// Matrix View;
		// Matrix Projection;
		// Matrix ProjectionInverse;

		u32 MeshesCount;
		u32 TexturesCount;
		u32 LightsCount;
		u32 DecalsCount;
	};

	struct GPUScene
	{
		static constexpr int MaxMeshes = 65536;
		static constexpr int MaxGPULights = 8192;
		static constexpr int MaxDecals = 8192;

		AccelerationStructure* TLAS = nullptr;
		std::vector<GPUSceneMesh> Meshes;
		std::vector<Texture2*> Textures;
		std::vector<GPULight> Lights;
		std::vector<GPUDecal> Decals;

		std::vector<IrradianceVolume> IrradianceVolumes;

		GPUCamera MainCamera;
		bool Dirty = false;

		// GPUScene also owns GPU-specific resources
		Buffer* SceneBuffer = nullptr;
		Buffer* SceneUploadBuffers[MaxFramesInFlight] {nullptr};

		Buffer* LightsBuffer = nullptr;
		Buffer* LightUploadBuffers[MaxFramesInFlight] {nullptr};

		Buffer* MeshesBuffer = nullptr;
		Buffer* MeshesUploadBuffers[MaxFramesInFlight] {nullptr};

		Buffer* DecalsBuffers = nullptr;
		Buffer* DecalsUploadBuffers[MaxFramesInFlight] {nullptr};

		// TODO: materials

		GPUSceneCompact CreateCompact() const
		{
			return GPUSceneCompact {
				.MeshesCount = (u32)Meshes.size(),
				.TexturesCount = (u32)Textures.size(),
				.LightsCount = (u32)Lights.size(),
				.DecalsCount = (u32)Decals.size(),
			};
		}

		static GPUScene* CreateGPUScene(SPtr<DeviceVulkan> Device)
		{
			GPUScene* Result = new GPUScene();

			// scene
			{
				BufferDesc SceneBufferDesc;
				SceneBufferDesc.Size = sizeof(GPUSceneCompact);
				SceneBufferDesc.BindFlags = BufferType::UAV;
				Result->SceneBuffer = Device->CreateBuffer(SceneBufferDesc, nullptr);
				Device->SetDebugName(Result->SceneBuffer, "GPUScene.Scene");

				for (Buffer*& UploadBuffer : Result->SceneUploadBuffers)
				{
					BufferDesc UploadBufferDesc(sizeof(GPUSceneCompact), BufferType::UAV);
					UploadBufferDesc.HostVisible = true;
					UploadBuffer = Device->CreateBuffer(UploadBufferDesc, nullptr);
					Device->SetDebugName(UploadBuffer, "GPUScene.Scene (upload buffer)");
				}
			}

			// lights
			{
				BufferDesc LightBufferDesc;
				LightBufferDesc.Size = GPUScene::MaxGPULights * sizeof(GPULight);
				LightBufferDesc.BindFlags = BufferType::UAV;
				Result->LightsBuffer = Device->CreateBuffer(LightBufferDesc, nullptr);
				Device->SetDebugName(Result->LightsBuffer, "GPUScene.Lights");

				for (Buffer*& UploadBuffer : Result->LightUploadBuffers)
				{
					BufferDesc UploadBufferDesc(GPUScene::MaxGPULights * sizeof(GPULight), BufferType::UAV);
					UploadBufferDesc.HostVisible = true;
					UploadBuffer = Device->CreateBuffer(UploadBufferDesc, nullptr);
					Device->SetDebugName(UploadBuffer, "GPUScene.Lights (upload buffer)");
				}
			}

			// meshes
			{
				BufferDesc MeshBufferDesc;
				MeshBufferDesc.Size = GPUScene::MaxMeshes * sizeof(GPUSceneMeshCompact);
				MeshBufferDesc.BindFlags = BufferType::UAV;
				Result->MeshesBuffer = Device->CreateBuffer(MeshBufferDesc, nullptr);
				Device->SetDebugName(Result->MeshesBuffer, "GPUScene.Meshes");

				for (Buffer*& UploadBuffer : Result->MeshesUploadBuffers)
				{
					BufferDesc UploadBufferDesc(GPUScene::MaxMeshes * sizeof(GPUSceneMeshCompact), BufferType::UAV);
					UploadBufferDesc.HostVisible = true;
					UploadBuffer = Device->CreateBuffer(UploadBufferDesc, nullptr);
					Device->SetDebugName(UploadBuffer, "GPUScene.Meshes (upload buffer)");
				}
			}

			// materials
			{
				// TODO:
			}

			// decals
			{
				BufferDesc DecalsBufferDesc;
				DecalsBufferDesc.Size = GPUScene::MaxDecals * sizeof(GPUDecal);
				DecalsBufferDesc.BindFlags = BufferType::UAV;
				Result->DecalsBuffers = Device->CreateBuffer(DecalsBufferDesc, nullptr);
				Device->SetDebugName(Result->DecalsBuffers, "GPUScene.Decals");

				for (Buffer*& UploadBuffer : Result->DecalsUploadBuffers)
				{
					BufferDesc UploadBufferDesc(GPUScene::MaxDecals * sizeof(GPUDecal), BufferType::UAV);
					UploadBufferDesc.HostVisible = true;
					UploadBuffer = Device->CreateBuffer(UploadBufferDesc, nullptr);
					Device->SetDebugName(UploadBuffer, "GPUScene.Decals (upload buffer)");
				}
			}

			return Result;
		}

		static void DestroyGPUScene(GPUScene* Scene, SPtr<DeviceVulkan> Device)
		{
			Device->DestroyBuffer(Scene->LightsBuffer);

			for (Buffer*& UploadBuffer : Scene->LightUploadBuffers)
			{
				Device->DestroyBuffer(UploadBuffer);
			}

			Device->DestroyBuffer(Scene->DecalsBuffers);

			for (Buffer*& UploadBuffer : Scene->DecalsUploadBuffers)
			{
				Device->DestroyBuffer(UploadBuffer);
			}
		}
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
