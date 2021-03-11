#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/Box.h>

namespace Columbus
{

	struct Vertex
	{
		Vector3 pos;
		Vector2 UV;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
	};

	struct SubModel
	{
		bool Indexed = false;
		uint32 VerticesCount = 0;
		uint32 IndicesCount = 0;

		Vector3* Positions = nullptr;
		Vector2* UVs = nullptr;
		Vector3* Normals = nullptr;
		Vector3* Tangents = nullptr;
		int*     Indices = nullptr;
		int IndexSize = 4;

		Box BoundingBox;
		bool Bounded = false;

		void FreeData()
		{
			Indexed = false;
			VerticesCount = 0;
			IndicesCount = 0;
			delete[] Positions;
			delete[] UVs;
			delete[] Normals;
			delete[] Tangents;
			delete[] Indices;
			Positions = nullptr;
			UVs = nullptr;
			Normals = nullptr;
			Tangents = nullptr;
			Indices = nullptr;
			IndexSize = 4;
			BoundingBox = {};
			Bounded = false;
		}
	};

	class ModelLoader
	{
	public:
		std::vector<SubModel> SubModels;
	public:
		virtual bool Load(const char* File) = 0;
		virtual ~ModelLoader() {}
	};

	class Model
	{
	private:
		Box BoundingBox;

		std::vector<SubModel> SubModels;
	public:
		Model();

		bool Load(const char* File);
		void FreeData();
		void RecalculateBounds();
		void RecalculateTangents();

		uint32 GetSubModelsCount() const { return SubModels.size();  }
		bool HasSubMeshes() const { return SubModels.empty(); }
		const SubModel& GetSubModel(int index) const { return SubModels[index]; }
		Box GetBoundingBox() const { return BoundingBox; }
		
		~Model();
	};

}


