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

	struct HeaderCMFNew
	{
		uint8  Magic[24];
		uint32 Version;
		uint32 Filesize;
		uint32 Flags;
		uint32 Compression;
		uint32 NumVertices;
		uint32 NumArrays;
	};

	enum CMFCompression
	{
		CMF_COMPRESSION_NONE = 1 << 0,
		CMF_COMPRESSION_ZSTD = 1 << 1
	};

	enum CMFType
	{
		CMF_TYPE_POSITION = 0,
		CMF_TYPE_TEXCOORD = 1,
		CMF_TYPE_NORMAL = 2,
		CMF_TYPE_TANGENT = 3,
		CMF_TYPE_COLOR = 4,
		CMF_TYPE_INDICES = 5
	};

	enum CMFFormat
	{
		CMF_FORMAT_BYTE = 0,
		CMF_FORMAT_UBYTE = 1,
		CMF_FORMAT_SHORT = 2,
		CMF_FORMAT_USHORT = 3,
		CMF_FORMAT_INT = 4,
		CMF_FORMAT_UINT = 5,
		CMF_FORMAT_HALF = 6,
		CMF_FORMAT_FLOAT = 7,
		CMF_FORMAT_DOUBLE = 8
	};

	void ConvertToCompressed(const char* Old, const char* New)
	{
		File OldModel(Old, "rb");
		File NewModel(New, "wb");

		if (!OldModel.IsOpened()) return;
		if (!NewModel.IsOpened()) return;

		HeaderCMFNew Header;
		OldModel.Read(Header);

		uint32 DataSize = OldModel.GetSize() - sizeof(Header);

		uint8* SourceFile = new uint8[DataSize];
		OldModel.ReadBytes(SourceFile, DataSize);

		uint64 Bound = ZSTD_compressBound(DataSize);
		uint8* DestinyFile = new uint8[Bound];
		uint64 DestinySize = ZSTD_compress(DestinyFile, Bound, SourceFile, DataSize, 1);

		Header.Compression = CMF_COMPRESSION_ZSTD;
		Header.Filesize = DestinySize + sizeof(Header);

		NewModel.Write(Header);
		NewModel.WriteBytes(DestinyFile, DestinySize);

		delete[] SourceFile;
		delete[] DestinyFile;
	}

	template <typename T>
	static T* CreateAndFill(uint32 Count, void* Data, uint64 Size)
	{
		T* Result = new T[Count];
		
		memcpy(Result, Data, Size);
		return Result;
	}

	ModelLoaderCMF::ModelLoaderCMF() {}

	bool IsNew(const char* FileName)
	{
		File Model(FileName, "rb");
		if (!Model.IsOpened()) return false;

		uint8 Magic[24];
		Model.Read(Magic);
		return memcmp(Magic, "COLUMBUS MODEL FORMAT  \0", sizeof(Magic)) == 0;
	}

	bool ModelLoaderCMF::Load(const char* FileName)
	{
		if (IsNew(FileName))
		{
			File Model(FileName, "rb");
			if (!Model.IsOpened()) return false;

			HeaderCMFNew Header;
			Model.Read(Header);

			uint32 Offset = 0;
			uint8* Data = nullptr;

			if (Header.Compression == CMF_COMPRESSION_NONE)
			{
				uint64 Size = Model.GetSize() - sizeof(HeaderCMFNew);
				Data = new uint8[Size];
				Model.ReadBytes(Data, Size);
				Model.Close();
			}
			else if (Header.Compression == CMF_COMPRESSION_ZSTD)
			{
				uint32 FileDataSize = Model.GetSize() - sizeof(HeaderCMFNew);
				uint8* FileData = new uint8[FileDataSize];
				Model.ReadBytes(FileData, FileDataSize);

				uint64 DecompressedSize = ZSTD_getDecompressedSize(FileData, Model.GetSize() - sizeof(HeaderCMFNew));
				uint8* Decompressed = new uint8[DecompressedSize];
				ZSTD_decompress(Decompressed, DecompressedSize, FileData, Model.GetSize() - sizeof(HeaderCMFNew));
				delete[] FileData;

				Data = Decompressed;
			}

			if (Data == nullptr) return false;

			VerticesCount = Header.NumVertices;

			for (int i = 0; i < Header.NumArrays; i++)
			{
				struct HeaderArray
				{
					uint32 Type;
					uint32 Format;
					uint32 Size;
				} ArrayHeader;

				ArrayHeader = *((HeaderArray*)(Data + Offset));
				Offset += sizeof(HeaderArray);

				switch (ArrayHeader.Type)
				{
				case CMF_TYPE_POSITION: Positions = CreateAndFill<Vector3>(VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_TEXCOORD: UVs       = CreateAndFill<Vector2>(VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_NORMAL:   Normals   = CreateAndFill<Vector3>(VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_TANGENT:  Tangents  = CreateAndFill<Vector3>(VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_COLOR: break;
				case CMF_TYPE_INDICES:
					switch (ArrayHeader.Format)
					{
					case CMF_FORMAT_BYTE:
					case CMF_FORMAT_UBYTE: IndexSize = 1; break;

					case CMF_FORMAT_SHORT:
					case CMF_FORMAT_USHORT: IndexSize = 2; break;

					case CMF_FORMAT_INT:
					case CMF_FORMAT_UINT: IndexSize = 4; break;
					}

					IndicesCount = ArrayHeader.Size / IndexSize;
					Indexed = true;

					switch (IndexSize)
					{
					case 1: Indices = CreateAndFill<uint8> (IndicesCount, Data + Offset, ArrayHeader.Size); break;
					case 2: Indices = CreateAndFill<uint16>(IndicesCount, Data + Offset, ArrayHeader.Size); break;
					case 4: Indices = CreateAndFill<uint32>(IndicesCount, Data + Offset, ArrayHeader.Size); break;
					}

					break;
				}

				Offset += ArrayHeader.Size;
			}

			//delete[] Data;

			for (int i = 0; i < VerticesCount; i++)
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
		else
		{
			File Model(FileName, "rb");
			if (!Model.IsOpened()) return false;

			HeaderCMF Header;
			Model.Read(Header.Magic);
			Model.Read(Header.Count);
			Model.Read(Header.Compression);

			Positions = new Vector3[Header.Count * 3];
			UVs = new Vector2[Header.Count * 3];
			Normals = new Vector3[Header.Count * 3];

			Indexed = false;
			VerticesCount = Header.Count * 3;
			IndicesCount = 0;

			if (Header.Compression == 0x00)
			{
				Model.ReadBytes(Positions, Header.Count * 3 * 3 * sizeof(float));
				Model.ReadBytes(UVs, Header.Count * 3 * 2 * sizeof(float));
				Model.ReadBytes(Normals, Header.Count * 3 * 3 * sizeof(float));
			}
			else if (Header.Compression == 0xFF)
			{
				uint8* FileData = new uint8[Model.GetSize()];
				Model.ReadBytes(FileData, Model.GetSize());

				uint64 DecompressedSize = ZSTD_getDecompressedSize(FileData, Model.GetSize() - 26);
				uint8* Decompressed = new uint8[DecompressedSize];
				ZSTD_decompress(Decompressed, DecompressedSize, FileData, Model.GetSize() - 26);

				memcpy(Positions, Decompressed, Header.Count * sizeof(float) * 3 * 3); Decompressed += Header.Count * sizeof(float) * 3 * 3;
				memcpy(UVs, Decompressed, Header.Count * sizeof(float) * 3 * 2); Decompressed += Header.Count * sizeof(float) * 3 * 2;
				memcpy(Normals, Decompressed, Header.Count * sizeof(float) * 3 * 3); Decompressed += Header.Count * sizeof(float) * 3 * 3;

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
		}

		return true;
	}

	bool ModelLoaderCMF::IsCMF(const char* FileName)
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



