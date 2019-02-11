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

	class ModelLoader
	{
	public:
		bool Indexed = false;

		uint32 VerticesCount = 0;
		uint32 IndicesCount = 0;
		uint32 IndexSize = 0;

		Vector3* Positions = nullptr;
		Vector2* UVs = nullptr;
		Vector3* Normals = nullptr;
		Vector3* Tangents = nullptr;
		void*    Indices = nullptr;

		Box BoundingBox;
	public:
		ModelLoader() {}

		virtual bool Load(const char* File) { return false; }

		~ModelLoader() {}
	};

	class Model
	{
	private:
		bool Indexed = false;
		bool Exist = false;

		uint32 VerticesCount = 0;
		uint32 IndicesCount  = 0;
		uint32 IndexSize = 0;

		Vector3* Positions = nullptr;
		Vector2* UVs = nullptr;
		Vector3* Normals = nullptr;
		Vector3* Tangents = nullptr;

		Vertex* Vertices = nullptr;
		void* Indices = nullptr;

		Box BoundingBox;
	public:
		Model();

		bool Load(const char* File);
		void FreeData();

		bool IsIndexed() const { return Indexed; }
		bool IsExist()   const { return Exist;   }

		uint32 GetVerticesCount() const { return VerticesCount; }
		uint32 GetIndicesCount()  const { return IndicesCount;  }
		uint32 GetIndexSize()     const { return IndexSize;     }

		bool HasPositions() const { return Positions != nullptr; }
		bool HasUVs()       const { return UVs       != nullptr; }
		bool HasNormals()   const { return Normals   != nullptr; }
		bool HasTangents()  const { return Tangents  != nullptr; }
		bool HasVertices()  const { return Vertices  != nullptr; }
		bool HasIndices()   const { return Indices   != nullptr; }

		const Vector3* GetPositions() const { return Positions; }
		const Vector2* GetUVs()       const { return UVs;       }
		const Vector3* GetNormals()   const { return Normals;   }
		const Vector3* GetTangents()  const { return Tangents;  }
		const Vertex*  GetVertices()  const { return Vertices;  }
		const void*    GetIndices()   const { return Indices;   }

		Box GetBoundingBox() const { return BoundingBox; }
		
		~Model();
	};

}


