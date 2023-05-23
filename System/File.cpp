#include <System/File.h>
#include <Core/Assert.h>
#include <Core/Platform.h>
#include <utility>
#include <cstring>
#include <cstdio>

#if defined(PLATFORM_WINDOWS)
	#define FSeek64 _fseeki64
	#define FTell64 _ftelli64
#else
	#define FSeek64 fseeko64
	#define FTell64 ftello64
#endif

namespace Columbus
{
	
	File::File() {}
	
	File::File(File& Other)
	{
		Handle = Other.Handle;
		FileSize = Other.FileSize;
		FileName = Other.FileName;

		Other.Handle = nullptr;
		Other.FileSize = 0;
		Other.FileName = nullptr;
	}

	File::File(const char* File, const char* Modes)
	{
		Open(File, Modes);
	}

	File& File::operator=(File& Other)
	{
		if (this == &Other) return *this;

		std::swap(Handle, Other.Handle);
		std::swap(FileSize, Other.FileSize);
		std::swap(FileName, Other.FileName);

		return *this;
	}

	File& File::operator<<(const char Char)
	{
		Write(Char);
		return *this;
	}

	File& File::operator<<(const char* String)
	{
		WriteBytes(String, strlen(String));
		return *this;
	}

	String File::ReadAllText(const char* Filename)
	{
		File F(Filename, "rt");
		String Result(F.GetSize(), '\0');
		F.Read(&Result[0], F.GetSize(), 1);
		return Result;
	}

	bool File::Open(const char* File, const char* Modes)
	{
		Handle = fopen(File, Modes);
		if (Handle != nullptr)
		{
			int Len = strlen(File) + 1;
			FileName = new char[Len];
			memcpy(FileName, File, Len);

			FSeek64(Handle, 0, SEEK_END);
			FileSize = FTell64(Handle);
			FSeek64(Handle, 0, SEEK_SET);

			return true;
		}

		return false;
	}

	bool File::Close()
	{
		bool ret = false;

		if (Handle != nullptr)
		{
			ret = fclose(Handle) == 0;
			Handle = nullptr;
		}
		
		delete[] FileName;
		FileName = nullptr;
		return ret;
	}

	const char* File::GetName() const
	{
		return FileName != nullptr ? FileName : "";
	}

	uint64 File::GetSize() const
	{
		if (Handle == nullptr) return 0;

		return FileSize;
	}

	bool File::IsEOF() const
	{
		return (feof(Handle) != 0);
	}

	int File::Getc() const
	{
		return (fgetc(Handle) == 0);
	}

	bool File::SeekSet(uint64 Offset) const
	{
		CurrentOffset = Offset;
		return (FSeek64(Handle, Offset, SEEK_SET) == 0);
	}
	
	bool File::SeekEnd(uint64 Offset) const
	{
		CurrentOffset = FileSize - Offset;
		return (FSeek64(Handle, Offset, SEEK_END) == 0);
	}
	
	bool File::SeekCur(int64 Offset) const
	{
		CurrentOffset += Offset;
		return (FSeek64(Handle, Offset, SEEK_CUR) == 0);
	}
	
	uint64 File::Tell() const
	{
		if (Handle == nullptr) return 0;
		// FTell64 is MUCH slower than offset variable
		//return FTell64(Handle);
		return CurrentOffset;
	}
	
	bool File::Flush() const
	{
		if (Handle == nullptr) return false;
		return (fflush(Handle) == 0);
	}
	
	bool File::IsOpened() const
	{
		return (Handle != nullptr);
	}

	char* File::ReadLine(char* Buf, uint32 MaxCount) const
	{
		if (Handle == nullptr) return nullptr;
		fgets(Buf, MaxCount, Handle);
		CurrentOffset = FTell64(Handle);
		return Buf;
	}
	
	size_t File::Read(void* Data, size_t Size, size_t Packs) const
	{
		if (Handle == nullptr) return 0;
		CurrentOffset += Size * Packs;
		return fread(Data, Size, Packs, Handle);
	}

	size_t File::Write(const void* Data, size_t Size, size_t Packs) const
	{
		if (Handle == nullptr) return 0;
		CurrentOffset += Size * Packs;
		return fwrite(Data, Size, Packs, Handle);
	}

	bool File::ReadBytes(void* Data, uint64 Size)
	{
		if (!Data) return false;
		if (Handle == nullptr) return false;
		CurrentOffset += Size;
		if (fread(Data, Size, 1, Handle) != 1) return false;
		else return true;
	}

	bool File::WriteBytes(const void* Data, uint64 Size)
	{
		if (!Data) return false;
		if (Handle == nullptr) return false;
		CurrentOffset += Size;
		if (fwrite(Data, Size, 1, Handle) != 1) return false;
		else return true;
	}

	File::~File()
	{
		Close();
	}

}


