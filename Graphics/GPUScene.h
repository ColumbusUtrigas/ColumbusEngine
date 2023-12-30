#pragma once

#include "Graphics/Core/GraphicsCore.h"
#include "Camera.h"
#include "Graphics/Light.h"
#include "Graphics/Vulkan/DeviceVulkan.h"
#include "Profiling/Profiling.h"
#include <Core/Core.h>
#include <vector>

namespace Columbus
{

	// design idea:
	// the GPU scene is immutable during the rendering and is created as a "proxy" from
	// the CPU representation.
	// after update/upload of a GPU scene, it can be used during rendering in render graph
	// Similar concept as Unreal's RenderProxy, but simple

	// TODO: define GPUScene upload routine, which uploads data to the GPU via Device
	// TODO: define GPUScene update/populate routine, which converts CPU entities to GPU representation
	// TODO: implement Lights buffer upload first

	struct GPUSceneMesh
	{
		AccelerationStructure* BLAS;
		Matrix Transform;
		Buffer* Vertices;
		Buffer* Indices;
		Buffer* UVs;
		Buffer* Normals;
		Buffer* Material; // TODO
		uint32_t VertexCount;
		uint32_t IndicesCount;
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

	struct GPUDecal
	{
		Matrix Model;
		Matrix ModelInverse;
		Texture2* Texture;

		VkDescriptorSet _DescriptorSets[MaxFramesInFlight]{NULL};
	};

	struct GPUScene
	{
		static constexpr int MaxGPULights = 8192;
		static constexpr int MaxDecals = 8192;

		AccelerationStructure* TLAS = nullptr;
		std::vector<GPUSceneMesh> Meshes;
		std::vector<Texture2*> Textures;
		std::vector<GPULight> Lights;
		std::vector<GPUDecal> Decals;

		GPUCamera MainCamera;
		bool Dirty = false;

		// GPUScene also owns GPU-specific resources
		Buffer* LightsBuffer = nullptr;
		Buffer* LightUploadBuffers[MaxFramesInFlight] {nullptr};

		Buffer* DecalsBuffers = nullptr;
		Buffer* DecalsUploadBuffers[MaxFramesInFlight] {nullptr};

		// TODO: camera, lights, decals, materials, per-object data

		static GPUScene* CreateGPUScene(SPtr<DeviceVulkan> Device)
		{
			GPUScene* Result = new GPUScene();

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
