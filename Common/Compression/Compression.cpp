#include <Common/Compression/Compression.h>
#include <Core/Types.h>
#include <System/File.h>
#include <zstd.h>

namespace Columbus
{

	bool Compression::CompressFileToFileZSTD(std::string Source, std::string Destiny)
	{
		File SourceFile(Source, "rb");
		File DestinyFile(Destiny, "wb");
		if (!SourceFile.IsOpened()) return false;
		if (!DestinyFile.IsOpened()) return false;

		uint64 FileSize = SourceFile.GetSize();
		uint8* SourceData = new uint8[FileSize];
		SourceFile.ReadBytes(SourceData, FileSize);
		SourceFile.Close();

		uint64 Bound = ZSTD_compressBound(FileSize);
		uint8* DestinyData = new uint8[Bound];
		uint64 DestinySize = ZSTD_compress(DestinyData, Bound, SourceData, FileSize, 1);
		DestinyFile.WriteBytes(DestinyData, DestinySize);
		DestinyFile.Close();

		delete[] SourceData;
		delete[] DestinyData;
		return true;
	}

}













