#pragma once

#include "AccelerationStructure.h"
#include "Buffer.h"
#include "Texture.h"
#include "Camera.h"
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

		bool operator==(const GPUCamera& Other)
		{
			return Position == Other.Position && Direction == Other.Direction && Up == Other.Up && Side == Other.Side;
		}

		bool operator!=(const GPUCamera& Other) { return !(*this == Other); }
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

}
