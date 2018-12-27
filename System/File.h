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
		FILE* Handle = nullptr;
		uint64 FileSize = 0;
		std::string FileName;
	public:
		File();
		File(File& Other);
		File(const std::string& File, const std::string& Modes);

		File& operator=(File& Other);
		File& operator<<(const char Char);
		File& operator<<(const std::string& String);

		bool Open(const std::string& File, const std::string& Modes);
		bool Close();

		std::string GetName() const;
		uint64 GetSize() const;

		bool IsEOF() const;
		int Getc() const;
		bool SeekSet(uint64 Offset) const;
		bool SeekEnd(uint64 Offset) const;
		bool SeekCur(uint64 Offset) const;
		uint64 Tell() const;
		bool Flush() const;

		bool IsOpened() const;

		size_t Read(void* Data, size_t Size, size_t Packs) const;
		size_t Write(const void* Data, size_t Size, size_t Packs) const;

		bool ReadBytes(void* Data, uint64 Size);
		bool WriteBytes(const void* Data, uint64 Size);
		
		template <typename T>
		bool Read(T& Data)
		{
			return ReadBytes(&Data, sizeof(T));
		}

		template <typename T>
		bool Write(const T& Data)
		{
			return WriteBytes(&Data, sizeof(T));
		}

		~File();
	};

}







