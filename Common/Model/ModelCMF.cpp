/************************************************
*                   ModelCMF.cpp                *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   08.01.2018                  *
*************************************************/
#include <Common/Model/Model.h>
#include <System/File.h>

namespace Columbus
{

	typedef struct
	{
		uint8_t magic[21]; //Magic string "COLUMBUS MODEL FORMAT"
		uint32_t count;    //Polygons count
	} CMF_HEADER;

	static bool ReadHeader(CMF_HEADER* aHeader, File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->readBytes(aHeader->magic, 21)) return false;
		if (!aFile->readUint32(&aHeader->count)) return false;

		return true;
	}

	bool ModelIsCMF(const std::string aFile)
	{
		File file(aFile, "rb");
		if (!file.isOpened()) return false;

		const char* magic = "COLUMBUS MODEL FORMAT";
		uint8_t fmagic[21];
		bool ret = true;

		file.read(fmagic, 21, 1);
		file.close();

		for (int i = 0; i < 21; i++)
			if (magic[i] != fmagic[i]) ret = false;

		return ret;
	}

	std::vector<Vertex> ModelLoadCMF(const std::string aFile)
	{
		std::vector<Vertex> ret;
		size_t i, j;
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;

		File file(aFile, "rb");
		if (!file.isOpened()) return ret;

		CMF_HEADER header;
		if (!ReadHeader(&header, &file)) return ret;

		float* vbuffer = new float[header.count * 3 * 3]; //Vertex buffer
		float* ubuffer = new float[header.count * 3 * 2]; //TexCoord buffer
		float* nbuffer = new float[header.count * 3 * 3]; //Normal buffer

		if (!file.readBytes(vbuffer, header.count * sizeof(float) * 3 * 3)) return ret;
		if (!file.readBytes(ubuffer, header.count * sizeof(float) * 3 * 2)) return ret;
		if (!file.readBytes(nbuffer, header.count * sizeof(float) * 3 * 3)) return ret;

		Vertex vert[3];
		Vector3 deltaPos1, deltaPos2;
		Vector2 deltaUV1, deltaUV2;
		Vector3 tangent, bitangent;
		float r;

		for (i = 0; i < header.count; i++)
		{
			for (j = 0; j < 3; j++)
			{
				vert[j].pos.x = vbuffer[vcounter++];
				vert[j].pos.y = vbuffer[vcounter++];
				vert[j].pos.z = vbuffer[vcounter++];

				vert[j].UV.x = ubuffer[ucounter++];
				vert[j].UV.y = ubuffer[ucounter++];

				vert[j].normal.x = nbuffer[ncounter++];
				vert[j].normal.y = nbuffer[ncounter++];
				vert[j].normal.z = nbuffer[ncounter++];
			}

			deltaPos1 = vert[1].pos - vert[0].pos;
			deltaPos2 = vert[2].pos - vert[0].pos;

			deltaUV1 = vert[1].UV - vert[0].UV;
			deltaUV2 = vert[2].UV - vert[0].UV;

			r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
			bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

			for (j = 0; j < 3; j++)
			{
				vert[j].tangent = tangent;
				vert[j].bitangent = bitangent;
				ret.push_back(vert[j]);
			}
		}

		if (vbuffer != nullptr)
			free(vbuffer);

		if (ubuffer != nullptr)
			free(ubuffer);

		if (nbuffer != nullptr)
			free(nbuffer);

		return ret;
	}

}



