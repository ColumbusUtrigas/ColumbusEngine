#pragma once

#include <vector>
#include <Math/Vector2.h>
#include <Math/Vector3.h>

namespace Columbus
{

	// all geometry is indexed
	struct CPUSceneMesh
	{
		std::vector<Vector3> Vertices;
		std::vector<Vector3> Normals;
		std::vector<Vector2> UV1;
		std::vector<Vector2> UV2;
		std::vector<u32> Indices;

		// TODO: trasformation
	};

	struct CPUScene
	{
		// 1 to 1 correspondance with GPUScene
		std::vector<CPUSceneMesh> Meshes;
	};

}
