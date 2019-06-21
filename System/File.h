#pragma once

#include <Core/Types.h>
#include <cstdio>

namespace Columbus
{

	class File
	{
	private:
		FILE* Handle = nullptr;
		uint64 FileSize = 0;
		mutable uint64 CurrentOffset = 0;
		char* FileName = nullptr;
	public:
		File();
		File(File& Other);
		File(const char* File, const char* Modes);

		File& operator=(File& Other);
		File& operator<<(const char Char);
		File& operator<<(const char* String);

		bool Open(const char* File, const char* Modes);
		bool Close();

		const char* GetName() const;
		uint64 GetSize() const;

		bool IsEOF() const;
		int Getc() const;
		bool SeekSet(uint64 Offset) const;
		bool SeekEnd(uint64 Offset) const;
		bool SeekCur(uint64 Offset) const;
		uint64 Tell() const;
		bool Flush() const;

		bool IsOpened() const;

		char* ReadLine(char* Buf, uint32 MaxCount) const;

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







