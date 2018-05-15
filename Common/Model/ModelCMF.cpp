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

		if (!aFile->ReadBytes(aHeader->magic, 21)) return false;
		if (!aFile->ReadUint32(&aHeader->count)) return false;

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
		File CMFModelFile(FileName, "rb");
		if (!CMFModelFile.IsOpened()) return false;

		uint8 Magic[21];
		CMFModelFile.ReadBytes(Magic, sizeof(Magic));
		CMFModelFile.Close();

		if (Memory::Memcmp(Magic, "COLUMBUS MODEL FORMAT", 21) == 0)
		{
			return true;
		}

		return false;
	}

	uint32 ModelLoadCMF(std::string FileName, std::vector<Vertex>& OutVertices)
	{
		File CMFModelFile(FileName, "rb");
		if (!CMFModelFile.IsOpened()) return 0;

		uint64 FileSize = CMFModelFile.GetSize();
		uint8* FileData = new uint8[FileSize];

		CMFModelFile.ReadBytes(FileData, FileSize);

		uint32 Count = ModelLoadCMFFromMemory(FileData, FileSize, OutVertices);
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
		uint8 Compressed = 0;
		Memory::Memcpy(&Count, FileData, sizeof(uint32));     FileData += sizeof(uint32);
		Memory::Memcpy(&Compressed, FileData, sizeof(uint8)); FileData += sizeof(uint8);

		float* vbuffer = new float[Count * 3 * 3]; //Vertex buffer
		float* ubuffer = new float[Count * 3 * 2]; //TexCoord buffer
		float* nbuffer = new float[Count * 3 * 3]; //Normal buffer

		if (Compressed == 0x00)
		{
			Memory::Memcpy(vbuffer, FileData, Count * sizeof(float) * 3 * 3); FileData += Count * sizeof(float) * 3 * 3;
			Memory::Memcpy(ubuffer, FileData, Count * sizeof(float) * 3 * 2); FileData += Count * sizeof(float) * 3 * 2;
			Memory::Memcpy(nbuffer, FileData, Count * sizeof(float) * 3 * 3); FileData += Count * sizeof(float) * 3 * 3;
		}
		else if (Compressed == 0xFF)
		{
			uint64 DecompressedSize = ZSTD_getDecompressedSize(FileData, FileSize - 26);
			uint8* Decompressed = new uint8[DecompressedSize];
			ZSTD_decompress(Decompressed, DecompressedSize, FileData, FileSize - 26);

			Memory::Memcpy(vbuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;
			Memory::Memcpy(ubuffer, Decompressed, Count * sizeof(float) * 3 * 2); Decompressed += Count * sizeof(float) * 3 * 2;
			Memory::Memcpy(nbuffer, Decompressed, Count * sizeof(float) * 3 * 3); Decompressed += Count * sizeof(float) * 3 * 3;

			Decompressed -= DecompressedSize;

			delete[] Decompressed;
		}

		ProcessVertices(Count, vbuffer, ubuffer, nbuffer, OutVertices);

		delete[] vbuffer;
		delete[] ubuffer;
		delete[] nbuffer;

		return Count;
	}

	void ModelConvertCMFToCompressed(std::string SourceFileName, std::string DestinyFileName)
	{
		File CMFSourceModelFile(SourceFileName, "rb");
		File CMFDestinyModelFile(DestinyFileName, "wb");
		if (!CMFSourceModelFile.IsOpened()) return;
		if (!CMFDestinyModelFile.IsOpened()) return;

		uint8* Header = new uint8[26];

		uint64 FileSize = CMFSourceModelFile.GetSize();
		uint8* SourceFile = new uint8[FileSize - 26];
		CMFSourceModelFile.ReadBytes(Header, 26);
		CMFSourceModelFile.ReadBytes(SourceFile, FileSize);
		CMFSourceModelFile.Close();

		uint64 Bound = ZSTD_compressBound(FileSize - 26);
		uint8* DestinyFile = new uint8[Bound];
		uint64 DestinySize = ZSTD_compress(DestinyFile, Bound, SourceFile, FileSize - 26, 1);
		CMFDestinyModelFile.WriteBytes(Header, 25);
		CMFDestinyModelFile.WriteUint8(0xFF);
		CMFDestinyModelFile.WriteBytes(DestinyFile, DestinySize);
		CMFDestinyModelFile.Close();

		delete[] SourceFile;
		delete[] DestinyFile;
	}

}



