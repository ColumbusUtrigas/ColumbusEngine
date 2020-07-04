#include <Common/Model/OBJ/ModelOBJ.h>
#include <cstdio>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION
#include <Lib/tinyobjloader/tiny_obj_loader.h>

namespace Columbus
{

	bool ModelLoaderOBJ::Load(const char* FileName)
	{
		tinyobj::attrib_t attrib; 
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn, err;

		bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FileName);
		if (!warn.empty()) Log::Warning("tinyobj: %s", warn.c_str());
		if (!err.empty()) Log::Error("tinyobj: %s", err.c_str());

		if (!res) return false;

		SubModels.resize(shapes.size());

		for (int i = 0; i < shapes.size(); i++)
		{
			SubModels[i].VerticesCount = shapes[i].mesh.indices.size();
			SubModels[i].Positions = new Vector3[SubModels[i].VerticesCount];

			for (int j = 0; j < SubModels[i].VerticesCount; j++)
			{
				auto vid = shapes[i].mesh.indices[j].vertex_index;
				SubModels[i].Positions[j].X = attrib.vertices[3.0 * vid + 0];
				SubModels[i].Positions[j].Y = attrib.vertices[3.0 * vid + 1];
				SubModels[i].Positions[j].Z = attrib.vertices[3.0 * vid + 2];
			}

			if (!attrib.texcoords.empty())
			{
				SubModels[i].UVs = new Vector2[SubModels[i].VerticesCount];
				for (int j = 0; j < SubModels[i].VerticesCount; j++)
				{
					auto vid = shapes[i].mesh.indices[j].texcoord_index;
					if (vid == -1)
						vid = j;

					SubModels[i].UVs[j].X = attrib.texcoords[2.0 * vid + 0];
					SubModels[i].UVs[j].Y = attrib.texcoords[2.0 * vid + 1];
				}
			}

			if (!attrib.normals.empty())
			{
				SubModels[i].Normals = new Vector3[SubModels[i].VerticesCount];
				for (int j = 0; j < SubModels[i].VerticesCount; j++)
				{
					auto vid = shapes[i].mesh.indices[j].normal_index;
					if (vid == -1)
						vid = j;
					SubModels[i].Normals[j].X = attrib.normals[3.0 * vid + 0];
					SubModels[i].Normals[j].Y = attrib.normals[3.0 * vid + 1];
					SubModels[i].Normals[j].Z = attrib.normals[3.0 * vid + 2];
				}
			}
		}

		return true;
	}

	bool ModelLoaderOBJ::IsOBJ(const char* FileName)
	{
		auto len = strlen(FileName);
		if (len > 4)
		{
			auto ptr = FileName + len - 4;
			bool is = strcmp(ptr, ".obj") == 0;
			return is;
		}

		return false;
	}

}
