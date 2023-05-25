#pragma once

#include "AccelerationStructure.h"
#include "Buffer.h"
#include "Texture.h"
#include "Camera.h"
#include <Core/Core.h>
#include <vector>

namespace Columbus
{

	struct GPUSceneMesh
	{
		AccelerationStructure* BLAS;
		Buffer* Vertices;
		Buffer* Indices;
		Buffer* UVs;
		Buffer* Normals;
		Buffer* Material; // TODO
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
	};

	// TODO: abstract as a GPU parameter struct?
	struct GPUScene
	{
		AccelerationStructure* TLAS;
		std::vector<GPUSceneMesh> Meshes;
		std::vector<Texture2*> Textures;
		std::vector<GPULight> Lights;

		GPUCamera MainCamera;

		bool Dirty = false;

		// TODO: camera, lights, decals, materials
	};

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
