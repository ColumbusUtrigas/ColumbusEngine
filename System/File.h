#pragma once

#include <cstdio>
#include <string>

#include <System/Assert.h>
#include <Core/Types.h>

namespace Columbus
{

	class File
	{
	private:
		FILE* Handle;
		uint64 FileSize;
		std::string FileName;
	public:
		File();
		File(File& Other);
		File(std::string File, std::string Modes);

		File& operator=(File& Other);
		File& operator<<(const char Char);
		File& operator<<(std::string String);

		bool Open(std::string File, std::string Modes);
		bool Close();

		std::string GetName() const;
		uint64 GetSize() const;

		bool IsEOF() const;
		int Getc() const;
		bool SeekSet(long int aOffset) const;
		bool SeekEnd(long int aOffset) const;
		bool SeekCur(long int aOffset) const;
		int Tell() const;
		bool Flush() const;

		bool IsOpened() const;

		size_t Read(void* aData, size_t aSize, size_t aPacks) const;
		size_t Write(const void* aData, size_t aSize, size_t aPacks) const;

		//Read batch of data from file
		bool ReadBytes(void* Data, uint64 Size);
		bool ReadUint8(uint8* Data);
		bool ReadInt8(int8* Data);
		bool ReadUint16(uint16* Data);
		bool ReadInt16(int16* Data);
		bool ReadUint32(uint32* Data);
		bool ReadInt32(int32* Data);

		//Write batch of data into file
		bool WriteBytes(const void* Data, uint64 Size);
		bool WriteUint8(const uint8 Data);
		bool WriteInt8(const int8 Data);
		bool WriteUint16(const uint16 Data);
		bool WriteInt16(const int16 Data);
		bool WriteUint32(const uint32 Data);
		bool WriteInt32(const int32 Data);

		~File();
	};

}







