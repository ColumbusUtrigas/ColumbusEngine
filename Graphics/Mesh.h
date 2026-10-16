#pragma once

#include <Math/Box.h>
#include <Graphics/Core/GraphicsCore.h>
#include <Animation/Skeleton.h>
#include <Core/Asset.h>
#include <Graphics/Material.h>
#include <Core/Reflection.h>
#include <Physics/PhysicsShape.h>
#include <Scene/Transform.h>
#include <string>

namespace Columbus
{
	struct MeshImportSettings
	{
		bool GenerateTangents = true;
		float ImportScale = 1.0f;
		bool BuildBLAS = true;
	};

	struct MeshMorphTarget
	{
		std::vector<Vector3> PositionDeltas;
		std::vector<Vector3> NormalDeltas;
		std::vector<Vector3> TangentDeltas;
	};

	struct MeshMorph
	{
		std::string Name;
		float DefaultWeight = 0.0f;
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
		std::vector<MeshMorphTarget> MorphTargets;

		void CalculateNormals();
		void CalculateTangents();
		bool IsSkeletal() { return !BoneIndices.empty(); }
		bool HasMorphTargets() const { return !MorphTargets.empty(); }
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
		Buffer* MorphDeltas = nullptr;
		u32 VertexCount;
		u32 IndicesCount;
		u32 MorphTargetCount = 0;

		// Derived geometry metadata shared by instance shadow-caster bounds.
		Box              Bounds;
		std::vector<Box> MorphPositionBounds;
		bool             HasBounds = false;
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
		AssetRef<Skeleton> Skeleton;
		MeshImportSettings ImportSettings;
		HCollisionSettings DefaultCollisionSettings;
		Box BoundingBox;

		std::vector<MeshSocket> Sockets;
		std::vector<MeshMorph> Morphs;
		std::vector<MeshPrimitive> Primitives;
	};
}

CREFLECT_DECLARE_STRUCT(Columbus::MeshImportSettings, 2, "C4E6BEF8-E86B-4DB5-9501-58A3DF57F324");
CREFLECT_DECLARE_STRUCT(Columbus::MeshMorphTarget, 2, "4DEB3A14-A0A6-48D4-85E7-1E4656E9C257");
CREFLECT_DECLARE_STRUCT(Columbus::MeshMorph, 1, "18F71438-0148-405D-BD89-60903955C21F");
CREFLECT_DECLARE_STRUCT(Columbus::CPUMeshResource, 3, "DF3E2144-55B9-4112-8A26-CA6DC6B7E5E1");
CREFLECT_DECLARE_STRUCT(Columbus::MeshPrimitive, 3, "BD5F2842-E2EF-4903-8C1D-7B4D6A698C63");
CREFLECT_DECLARE_STRUCT(Columbus::MeshSocket, 1, "EC8E7216-0E57-4FE2-9241-28127DD2A8AF");
CREFLECT_DECLARE_STRUCT(Columbus::Mesh2, 6, "7E656E4D-782F-4723-809F-0A12DC33FE99");
