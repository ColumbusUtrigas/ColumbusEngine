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

	template <typename T>
	static T* CreateAndFill(uint32 Count, void* Data, uint32 Size)
	{
		T* Result = new T[Count];
		
		memcpy((void*)Result, Data, Size);
		return Result;
	}

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
				uint32 Size = Model.GetSize() - sizeof(HeaderCMFNew);
				Data = new uint8[Size];
				Model.ReadBytes(Data, Size);
				Model.Close();
			}

			if (Data == nullptr) return false;

			SubModels.resize(1);

			SubModels[0].VerticesCount = Header.NumVertices;

			for (uint32 i = 0; i < Header.NumArrays; i++)
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
				case CMF_TYPE_POSITION: SubModels[0].Positions = CreateAndFill<Vector3>(SubModels[0].VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_TEXCOORD: SubModels[0].UVs       = CreateAndFill<Vector2>(SubModels[0].VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_NORMAL:   SubModels[0].Normals   = CreateAndFill<Vector3>(SubModels[0].VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_TANGENT:  SubModels[0].Tangents  = CreateAndFill<Vector3>(SubModels[0].VerticesCount, Data + Offset, ArrayHeader.Size); break;
				case CMF_TYPE_COLOR: break;
				case CMF_TYPE_INDICES:
					switch (ArrayHeader.Format)
					{
					case CMF_FORMAT_BYTE:
					case CMF_FORMAT_UBYTE: SubModels[0].IndexSize = 1; break;

					case CMF_FORMAT_SHORT:
					case CMF_FORMAT_USHORT: SubModels[0].IndexSize = 2; break;

					case CMF_FORMAT_INT:
					case CMF_FORMAT_UINT: SubModels[0].IndexSize = 4; break;
					}

					SubModels[0].IndicesCount = ArrayHeader.Size / SubModels[0].IndexSize;
					SubModels[0].Indexed = true;

					switch (SubModels[0].IndexSize)
					{
					case 1: SubModels[0].Indices = (int*)CreateAndFill<uint8> (SubModels[0].IndicesCount, Data + Offset, ArrayHeader.Size); break;
					case 2: SubModels[0].Indices = (int*)CreateAndFill<uint16>(SubModels[0].IndicesCount, Data + Offset, ArrayHeader.Size); break;
					case 4: SubModels[0].Indices = (int*)CreateAndFill<uint32>(SubModels[0].IndicesCount, Data + Offset, ArrayHeader.Size); break;
					}

					break;
				}

				Offset += ArrayHeader.Size;
			}

			delete[] Data;

			return true;
		}

		return false;
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

}


