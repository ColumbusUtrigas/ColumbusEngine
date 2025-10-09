#pragma once

#include <Math/Box.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Core/Reflection.h>

namespace Columbus
{

	// all geometry is indexed
	struct CPUMeshResource
	{
		std::vector<Vector3> Vertices;
		std::vector<Vector3> Normals;
		std::vector<Vector4> Tangents; // xyz - tangent, w - sign for bitangent
		std::vector<Vector2> UV1;
		std::vector<Vector2> UV2;
		std::vector<iVector4> BoneIndices;
		std::vector<Vector4>  BoneWeights;
		std::vector<u32> Indices;

		void CalculateTangents();
		bool IsSkeletal() { return !BoneIndices.empty(); }
	};

	struct GPUMeshResource
	{
		AccelerationStructure* BLAS;
		Buffer* Vertices;
		Buffer* Indices;
		Buffer* UV1;
		Buffer* UV2 = nullptr;
		Buffer* Normals;
		Buffer* Tangents;
		Buffer* BoneIndices = nullptr;
		Buffer* BoneWeights = nullptr;
		u32 VertexCount;
		u32 IndicesCount;
	};

	struct MeshPrimitive
	{
		Box BoundingBox;

		CPUMeshResource CPU;
		GPUMeshResource GPU;
	};

	struct Mesh2
	{
		Box BoundingBox;

		std::vector<MeshPrimitive> Primitives;
	};
}

CREFLECT_DECLARE_STRUCT(Columbus::Mesh2, 1, "7E656E4D-782F-4723-809F-0A12DC33FE99");
