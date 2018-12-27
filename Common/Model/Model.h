#pragma once

#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>
#include <Math/Box.h>
#include <vector>

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

	bool ModelIsCMF(std::string FileName);
	bool ModelIsCMFMemory(uint8* FileData, uint64 FileSize);

	uint32 ModelLoadCMF(std::string FileName, std::vector<Vertex>& OutVertices);
	uint32 ModelLoadCMFFromMemory(uint8* FileData, uint64 FileSize, std::vector<Vertex>& OutVertices);

	void ModelConvertCMFToCompressed(std::string SourceFileName, std::string DestinyFileName);

	class ModelLoader
	{
	public:
		bool Indexed = false;

		uint32 VerticesCount = 0;
		uint32 IndicesCount = 0;

		Vector3* Positions = nullptr;
		Vector2* UVs = nullptr;
		Vector3* Normals = nullptr;
		Vector3* Tangents = nullptr;
		uint32* Indices = nullptr;

		Box BoundingBox;
	public:
		ModelLoader() {}

		virtual bool Load(const std::string& File) { return false; }

		~ModelLoader() {}
	};

	class Model
	{
	private:
		bool Indexed = false;
		bool Exist = false;

		uint32 VerticesCount = 0;
		uint32 IndicesCount  = 0;

		Vector3* Positions = nullptr;
		Vector2* UVs = nullptr;
		Vector3* Normals = nullptr;
		Vector3* Tangents = nullptr;

		Vertex* Vertices = nullptr;
		uint32* Indices = nullptr;

		Box BoundingBox;
	public:
		Model();

		bool Load(const std::string& File);
		void FreeData();

		bool IsIndexed() const { return Indexed; }
		bool IsExist()   const { return Exist;   }

		uint32 GetVerticesCount() const { return VerticesCount; }
		uint32 GetIndicesCount()  const { return IndicesCount;  }

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
		const uint32*  GetIndices()   const { return Indices;   }

		Box GetBoundingBox() const { return BoundingBox; }
		
		~Model();
	};

}




