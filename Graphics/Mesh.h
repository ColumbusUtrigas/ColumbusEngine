#pragma once

#include <Math/Box.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Core/Asset.h>
#include <Core/Reflection.h>
#include <Physics/PhysicsShape.h>
#include <Scene/Transform.h>
#include <string>

namespace Columbus
{
	struct Material;

	struct MeshImportSettings
	{
		bool GenerateTangents = true;
		float ImportScale = 1.0f;
		bool BuildBLAS = true;
	};

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

		void CalculateNormals();
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
		std::string Name;
		AssetRef<Material> DefaultMaterial;

		Box BoundingBox;

		CPUMeshResource CPU;
		GPUMeshResource GPU;
	};

	struct MeshSocket
	{
		std::string Name;
		Transform LocalTransform;
	};

	struct Mesh2
	{
		std::string SourcePath;
		MeshImportSettings ImportSettings;
		HCollisionSettings DefaultCollisionSettings;
		Box BoundingBox;

		std::vector<MeshSocket> Sockets;
		std::vector<MeshPrimitive> Primitives;
	};
}

CREFLECT_DECLARE_STRUCT(Columbus::MeshImportSettings, 2, "C4E6BEF8-E86B-4DB5-9501-58A3DF57F324");
CREFLECT_DECLARE_STRUCT(Columbus::CPUMeshResource, 1, "DF3E2144-55B9-4112-8A26-CA6DC6B7E5E1");
CREFLECT_DECLARE_STRUCT(Columbus::MeshPrimitive, 2, "BD5F2842-E2EF-4903-8C1D-7B4D6A698C63");
CREFLECT_DECLARE_STRUCT(Columbus::MeshSocket, 1, "EC8E7216-0E57-4FE2-9241-28127DD2A8AF");
CREFLECT_DECLARE_STRUCT(Columbus::Mesh2, 3, "7E656E4D-782F-4723-809F-0A12DC33FE99");
