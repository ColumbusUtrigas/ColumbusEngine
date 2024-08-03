#pragma once

#include <Graphics/Light.h>
#include <Common/Model/Model.h>
#include <Math/Box.h>
#include <Graphics/Core/GraphicsCore.h>

namespace Columbus
{

	// TODO: remove
	class Mesh
	{
	protected:
		Box BoundingBox;
	public:
		std::vector<Mesh*> SubMeshes;
	public:
		Mesh() {}

		bool Load(const char* FileName)
		{
			Model TmpModel;

			if (!TmpModel.Load(FileName))
			{
				return false;
			}

			return Load(TmpModel);
		}

		virtual bool Load(const Model& InModel) = 0;

		// TODO: REMOVE
		virtual void Bind() = 0;
		virtual uint32 Render() = 0;
		virtual void Unbind() = 0;

		Box GetBoundingBox() const { return BoundingBox; }

		void Clear() {}

		virtual ~Mesh() {};
	};

	// all geometry is indexed
	struct CPUMeshResource
	{
		std::vector<Vector3> Vertices;
		std::vector<Vector3> Normals;
		std::vector<Vector4> Tangents; // xyz - tangent, w - sign for bitangent
		std::vector<Vector2> UV1;
		std::vector<Vector2> UV2;
		std::vector<u32> Indices;

		void CalculateTangents();
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
