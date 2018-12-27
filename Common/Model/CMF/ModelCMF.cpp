#include <Common/Model/CMF/ModelCMF.h>
#include <System/File.h>
#include <zstd.h>

namespace Columbus
{

	struct HeaderCMF
	{
		uint8 Magic[21];
		uint32 Count;
		uint8 Compression;
	};

	ModelLoaderCMF::ModelLoaderCMF() {}

	bool ModelLoaderCMF::Load(const std::string& FileName)
	{
		File Model(FileName, "rb");
		if (!Model.IsOpened()) return false;

		HeaderCMF Header;
		Model.Read(Header.Magic);
		Model.Read(Header.Count);
		Model.Read(Header.Compression);

		Positions = new Vector3[Header.Count * 3];
		UVs       = new Vector2[Header.Count * 3];
		Normals   = new Vector3[Header.Count * 3];

		Indexed = false;
		VerticesCount = Header.Count * 3;
		IndicesCount = 0;

		if (Header.Compression == 0x00)
		{
			Model.ReadBytes(Positions, Header.Count * 3 * 3 * sizeof(float));
			Model.ReadBytes(UVs,       Header.Count * 3 * 2 * sizeof(float));
			Model.ReadBytes(Normals,   Header.Count * 3 * 3 * sizeof(float));
		}
		else if (Header.Compression == 0xFF)
		{
			uint8* FileData = new uint8[Model.GetSize()];
			Model.ReadBytes(FileData, Model.GetSize());

			uint64 DecompressedSize = ZSTD_getDecompressedSize(FileData, Model.GetSize() - 26);
			uint8* Decompressed = new uint8[DecompressedSize];
			ZSTD_decompress(Decompressed, DecompressedSize, FileData, Model.GetSize() - 26);

			memcpy(Positions, Decompressed ,Header.Count * sizeof(float) * 3 * 3); Decompressed += Header.Count * sizeof(float) * 3 * 3;
			memcpy(UVs,       Decompressed, Header.Count * sizeof(float) * 3 * 2); Decompressed += Header.Count * sizeof(float) * 3 * 2;
			memcpy(Normals,   Decompressed, Header.Count * sizeof(float) * 3 * 3); Decompressed += Header.Count * sizeof(float) * 3 * 3;

			Decompressed -= DecompressedSize;

			delete[] Decompressed;
			delete[] FileData;
		}

		for (uint32 i = 0; i < Header.Count; i++)
		{
			if (Positions[i].X < BoundingBox.Min.X) BoundingBox.Min.X = Positions[i].X;
			if (Positions[i].X > BoundingBox.Max.X) BoundingBox.Max.X = Positions[i].X;
			if (Positions[i].Y < BoundingBox.Min.Y) BoundingBox.Min.Y = Positions[i].Y;
			if (Positions[i].Y > BoundingBox.Max.Y) BoundingBox.Max.Y = Positions[i].Y;
			if (Positions[i].Z < BoundingBox.Min.Z) BoundingBox.Min.Z = Positions[i].Z;
			if (Positions[i].Z > BoundingBox.Max.Z) BoundingBox.Max.Z = Positions[i].Z;
		}

		return true;
	}

	bool ModelLoaderCMF::IsCMF(const std::string& FileName)
	{
		File Model(FileName, "rb");
		if (!Model.IsOpened()) return false;

		uint8 Magic[21];
		Model.Read(Magic);
		Model.Close();

		if (memcmp(Magic, "COLUMBUS MODEL FORMAT", 21) == 0)
		{
			return true;
		}

		return false;
	}

	ModelLoaderCMF::~ModelLoaderCMF() {}

}



