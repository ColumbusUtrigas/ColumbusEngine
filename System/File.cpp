#include <System/File.h>

namespace Columbus
{

	bool FileReadBytes(void* aData, size_t aSize, FILE* aFile)
	{
		if (fread(aData, aSize, 1, aFile) != 1) return false;
		else return true;
	}
	
	bool FileReadUint8(uint8_t* aData, FILE* aFile)
	{
		return FileReadBytes(aData, sizeof(uint8_t), aFile);
	}

	
	File::File() :
		Handle(nullptr),
		FileSize(0)
	{}
	
	File::File(File& Other) :
		Handle(nullptr),
		FileSize(0)
	{
		Handle = Other.Handle;
		FileSize = Other.FileSize;
		FileName = Other.FileName;

		Other.Handle = nullptr;
		Other.FileSize = 0;
		Other.FileName.clear();
	}

	File::File(std::string File, std::string Modes) :
		Handle(nullptr),
		FileSize(0)
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

	File& File::operator<<(const char aChar)
	{
		Write(&aChar, sizeof(char), 1);
		return *this;
	}

	File& File::operator<<(const std::string aString)
	{
		Write(aString.c_str(), aString.size() * sizeof(char), 1);
		return *this;
	}

	bool File::Open(std::string aFile, std::string aModes)
	{
		Handle = fopen(aFile.c_str(), aModes.c_str());
		if (Handle != nullptr)
		{
			FileName = aFile;

			fseek(Handle, 0, SEEK_END);
			FileSize = ftell(Handle);
			fseek(Handle, 0, SEEK_SET);

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
		
		FileName.clear();
		return ret;
	}

	std::string File::GetName() const
	{
		return FileName;
	}

	uint64 File::GetSize() const
	{
		if (Handle == nullptr) return 0;

		return FileSize;
	}

	bool File::IsEOF() const
	{
		return (feof(Handle) == 0);
	}

	int File::Getc() const
	{
		return (fgetc(Handle) == 0);
	}

	bool File::SeekSet(long int aOffset) const
	{
		return (fseek(Handle, aOffset, SEEK_SET) == 0);
	}
	
	bool File::SeekEnd(long int aOffset) const
	{
		return (fseek(Handle, aOffset, SEEK_END) == 0);
	}
	
	bool File::SeekCur(long int aOffset) const
	{
		return (fseek(Handle, aOffset, SEEK_CUR) == 0);
	}
	
	int File::Tell() const
	{
		if (Handle == nullptr) return 0;
		return ftell(Handle);
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
	
	size_t File::Read(void* aData, size_t aSize, size_t aPacks) const
	{
		if (Handle == nullptr) return 0;
		return fread(aData, aSize, aPacks, Handle);
	}

	size_t File::Write(const void* aData, size_t aSize, size_t aPacks) const
	{
		if (Handle == nullptr) return 0;
		return fwrite(aData, aSize, aPacks, Handle);
	}

	bool File::ReadBytes(void* aData, uint64 aSize)
	{
		if (!aData) return false;
		if (Handle == nullptr) return false;
		if (fread(aData, aSize, 1, Handle) != 1) return false;
		else return true;
	}

	bool File::ReadUint8(uint8* Data)
	{
		return ReadBytes(Data, sizeof(uint8));
	}

	bool File::ReadInt8(int8* Data)
	{
		return ReadBytes(Data, sizeof(int8));
	}

	bool File::ReadUint16(uint16* Data)
	{
		return ReadBytes(Data, sizeof(uint16));
	}

	bool File::ReadInt16(int16* Data)
	{
		return ReadBytes(Data, sizeof(int16));
	}

	bool File::ReadUint32(uint32* Data)
	{
		return ReadBytes(Data, sizeof(uint32));
	}

	bool File::ReadInt32(int32* Data)
	{
		return ReadBytes(Data, sizeof(int32));
	}

	bool File::WriteBytes(const void* Data, uint64 Size)
	{
		if (!Data) return false;
		if (Handle == nullptr) return false;
		if (fwrite(Data, Size, 1, Handle) != 1) return false;
		else return true;
	}
	
	bool File::WriteUint8(const uint8 Data)
	{
		return WriteBytes(&Data, sizeof(uint8));
	}
	
	bool File::WriteInt8(const int8 Data)
	{
		return WriteBytes(&Data, sizeof(int8));
	}

	bool File::WriteUint16(const uint16 Data)
	{
		return WriteBytes(&Data, sizeof(uint16));
	}

	bool File::WriteInt16(const int16 Data)
	{
		return WriteBytes(&Data, sizeof(int16));
	}

	bool File::WriteUint32(const uint32 Data)
	{
		return WriteBytes(&Data, sizeof(uint32));
	}

	bool File::WriteInt32(const int32 Data)
	{
		return WriteBytes(&Data, sizeof(int32));
	}

	File::~File()
	{
		Close();
	}

}





