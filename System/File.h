#pragma once

#include <Core/Core.h>
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
		File(File& Other); // copy invalidates Other
		File(const char* File, const char* Modes);

		File& operator=(File& Other);
		File& operator<<(const char Char);
		File& operator<<(const char* String);

		static String ReadAllText(const char* Filename);

		bool Open(const char* File, const char* Modes);
		bool Close();

		const char* GetName() const;
		uint64 GetSize() const;

		bool IsEOF() const;
		int Getc() const;
		bool SeekSet(uint64 Offset) const;
		bool SeekEnd(uint64 Offset) const;
		bool SeekCur( int64 Offset) const;
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

	enum class DataStreamType
	{
		File,
		Memory,
	};

	// both from memory and from filesystem
	struct DataStream
	{
		static DataStream CreateFromFile(const char* FileName, const char* Modes);
		static DataStream CreateFromMemory(u8* Memory, u64 Size);

		u64 GetSize() const;

		void SeekSet(u64 Offset);
		void SeekCur(i32 Offset);

		bool IsEOF() const;

		size_t Read(void* Data, size_t Size, size_t Packs);
		size_t Write(const void* Data, size_t Size, size_t Packs);

		bool ReadBytes(void* Data, u64 Size);
		bool WriteBytes(const void* Data, u64 Size);

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

		DataStreamType GetType() const { return Type; }
		bool IsValid() const { return Valid; }

	public:
		DataStreamType Type;

		bool Valid = false;

		File F;

		u8* Memory = nullptr;
		u64 MemorySize = 0;

		u64 CurrentOffset = 0;
	};
}
