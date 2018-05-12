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

#include <zstd.h>

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

	static void ProcessVertices(uint32 Count, float* InVertexBuffer, float* InUVBuffer, float* InNormalBuffer, std::vector<Vertex>& OutVertices)
	{
		if (Count == 0 &&
		    InVertexBuffer == nullptr &&
		    InUVBuffer == nullptr &&
		    InNormalBuffer == nullptr)
		{
			return;
		}

		Vertex vert[3];
		Vector3 deltaPos1, deltaPos2;
		Vector2 deltaUV1, deltaUV2;
		Vector3 tangent, bitangent;
		float r;

		uint64 vcounter = 0;
		uint64 ucounter = 0;
		uint64 ncounter = 0;

		OutVertices.reserve(Count);

		for (uint32 i = 0; i < Count; i++)
		{
			for (uint32 j = 0; j < 3; j++)
			{
				vert[j].pos.X = InVertexBuffer[vcounter++];
				vert[j].pos.Y = InVertexBuffer[vcounter++];
				vert[j].pos.Z = InVertexBuffer[vcounter++];

				vert[j].UV.X = InUVBuffer[ucounter++];
				vert[j].UV.Y = InUVBuffer[ucounter++];

				vert[j].normal.X = InNormalBuffer[ncounter++];
				vert[j].normal.Y = InNormalBuffer[ncounter++];
				vert[j].normal.Z = InNormalBuffer[ncounter++];
			}

			deltaPos1 = vert[1].pos - vert[0].pos;
			deltaPos2 = vert[2].pos - vert[0].pos;

			deltaUV1 = vert[1].UV - vert[0].UV;
			deltaUV2 = vert[2].UV - vert[0].UV;

			r = 1.0f / (deltaUV1.X * deltaUV2.Y - deltaUV1.Y * deltaUV2.X);
			tangent = (deltaPos1 * deltaUV2.Y - deltaPos2 * deltaUV1.Y) * r;
			bitangent = (deltaPos2 * deltaUV1.X - deltaPos1 * deltaUV2.X) * r;

			for (uint32 j = 0; j < 3; j++)
			{
				vert[j].tangent = tangent;
				vert[j].bitangent = bitangent;
				OutVertices.push_back(vert[j]);
			}
		}
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

	uint32 ModelLoadCMF(std::string FileName, std::vector<Vertex>& OutVertices)
	{
		File CMFModelFile(FileName, "rb");
		if (!CMFModelFile.isOpened()) return 0;

		uint64 FileSize = CMFModelFile.getSize();
		uint8* FileData = new uint8[FileSize];

		CMFModelFile.readBytes(FileData, FileSize);

		uint32 Count = ModelLoadCMFFromMemory(FileData, FileSize, OutVertices);
		delete[] FileData;
		return Count;
	}

	uint32 ModelLoadCMFCompressed(std::string FileName, std::vector<Vertex>& OutVertices)
	{
		File CMFModelFile(FileName, "rb");
		if (!CMFModelFile.isOpened()) return 0;

		uint64 FileSize = CMFModelFile.getSize();
		uint8* FileData = new uint8[FileSize];

		CMFModelFile.readBytes(FileData, FileSize);

		uint32 Count = ModelLoadCMFCompressedFromMemory(FileData, FileSize, OutVertices);
		delete[] FileData;
		return Count;
	}

	uint32 ModelLoadCMFFromMemory(uint8* FileData, uint64 FileSize, std::vector<Vertex>& OutVertices)
	{
		if (FileData == nullptr &&
		    FileSize == 0)
		{
			return 0;
		}

		FileData += 21;
		uint32 Count = 0;
		Count = *reinterpret_cast<uint32*>(FileData);
		FileData += sizeof(uint32);

		float* vbuffer = new float[Count * 3 * 3]; //Vertex buffer
		float* ubuffer = new float[Count * 3 * 2]; //TexCoord buffer
		float* nbuffer = new float[Count * 3 * 3]; //Normal buffer

		Memory::Memcpy(vbuffer, FileData, Count * sizeof(float) * 3 * 3); FileData += Count * sizeof(float) * 3 * 3;
		Memory::Memcpy(ubuffer, FileData, Count * sizeof(float) * 3 * 2); FileData += Count * sizeof(float) * 3 * 2;
		Memory::Memcpy(nbuffer, FileData, Count * sizeof(float) * 3 * 3); FileData += Count * sizeof(float) * 3 * 3;

		ProcessVertices(Count, vbuffer, ubuffer, nbuffer, OutVertices);

		delete[] vbuffer;
		delete[] ubuffer;
		delete[] nbuffer;

		return Count;
	}

	uint32 ModelLoadCMFCompressedFromMemory(uint8* FileData, uint64 FileSize, std::vector<Vertex>& OutVertices)
	{
		if (FileData == nullptr &&
		    FileSize == 0)
		{
			return 0;
		}

		uint64 DecompressedSize = ZSTD_getDecompressedSize(FileData, FileSize);
		uint8* Decompressed = new uint8[DecompressedSize];
		ZSTD_decompress(Decompressed, DecompressedSize, FileData, FileSize);

		Decompressed += 21;
		uint32 Count = 0;
		Count = *reinterpret_cast<uint32*>(Decompressed);
		Decompressed += sizeof(uint32);

		float* vbuffer = new float[Count * 3 * 3]; //Vertex buffer
		float* ubuffer = new float[Count * 3 * 2]; //TexCoord buffer
		float* nbuffer = new float[Count * 3 * 3]; //Normal buffer

		Memory::Memcpy(vbuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;
		Memory::Memcpy(ubuffer, Decompressed, Count * sizeof(float) * 3 * 2); Decompressed += Count * sizeof(float) * 3 * 2;
		Memory::Memcpy(nbuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;

		ProcessVertices(Count, vbuffer, ubuffer, nbuffer, OutVertices);
		Decompressed -= DecompressedSize;

		delete[] vbuffer;
		delete[] ubuffer;
		delete[] nbuffer;
		delete[] Decompressed;

		return Count;
	}

	void ModelConvertCMFToCompressed(std::string SourceFileName, std::string DestinyFileName)
	{
		File CMFSourceModelFile(SourceFileName, "rb");
		File CMFDestinyModelFile(DestinyFileName, "wb");
		if (!CMFSourceModelFile.isOpened()) return;
		if (!CMFDestinyModelFile.isOpened()) return;

		uint64 FileSize = CMFSourceModelFile.getSize();
		uint8* SourceFile = new uint8[FileSize];
		CMFSourceModelFile.readBytes(SourceFile, FileSize);
		CMFSourceModelFile.close();

		uint64 Bound = ZSTD_compressBound(FileSize);
		char* DestinyFile = new char[Bound];
		uint64 DestinySize = ZSTD_compress(DestinyFile, Bound, SourceFile, FileSize, 1);
		CMFDestinyModelFile.writeBytes(DestinyFile, DestinySize);
		CMFDestinyModelFile.close();

		delete[] SourceFile;
		delete[] DestinyFile;
	}

}



