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
#include <Core/Types.h>
#include <Core/Memory.h>
#include <System/File.h>

namespace Columbus
{

	typedef struct
	{
		uint8 magic[21]; //Magic string "COLUMBUS MODEL FORMAT"
		uint32 count;    //Polygons count
	} CMF_HEADER;

	static bool ReadHeader(CMF_HEADER* aHeader, File* aFile)
	{
		if (aHeader == nullptr || aFile == nullptr) return false;

		if (!aFile->readBytes(aHeader->magic, 21)) return false;
		if (!aFile->readUint32(&aHeader->count)) return false;

		return true;
	}

	bool ModelIsCMF(std::string FileName)
	{
		File file(FileName, "rb");
		if (!file.isOpened()) return false;

		const char* magic = "COLUMBUS MODEL FORMAT";
		uint8 fmagic[21];
		bool ret = true;

		file.read(fmagic, 21, 1);
		file.close();

		for (int i = 0; i < 21; i++)
		{
			if (magic[i] != fmagic[i])
			{
				ret = false;
			}
		}

		return ret;
	}

	std::vector<Vertex> ModelLoadCMF(std::string FileName)
	{
		std::vector<Vertex> ret;
		size_t i, j;
		uint64_t vcounter = 0;
		uint64_t ucounter = 0;
		uint64_t ncounter = 0;

		File file(FileName, "rb");
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
				vert[j].pos.X = vbuffer[vcounter++];
				vert[j].pos.Y = vbuffer[vcounter++];
				vert[j].pos.Z = vbuffer[vcounter++];

				vert[j].UV.X = ubuffer[ucounter++];
				vert[j].UV.Y = ubuffer[ucounter++];

				vert[j].normal.X = nbuffer[ncounter++];
				vert[j].normal.Y = nbuffer[ncounter++];
				vert[j].normal.Z = nbuffer[ncounter++];
			}

			deltaPos1 = vert[1].pos - vert[0].pos;
			deltaPos2 = vert[2].pos - vert[0].pos;

			deltaUV1 = vert[1].UV - vert[0].UV;
			deltaUV2 = vert[2].UV - vert[0].UV;

			r = 1.0f / (deltaUV1.X * deltaUV2.Y - deltaUV1.Y * deltaUV2.X);
			tangent = (deltaPos1 * deltaUV2.Y - deltaPos2 * deltaUV1.Y) * r;
			bitangent = (deltaPos2 * deltaUV1.X - deltaPos1 * deltaUV2.X) * r;

			for (j = 0; j < 3; j++)
			{
				vert[j].tangent = tangent;
				vert[j].bitangent = bitangent;
				ret.push_back(vert[j]);
			}
		}

		delete[] vbuffer;
		delete[] ubuffer;
		delete[] nbuffer;

		return ret;
	}

}



