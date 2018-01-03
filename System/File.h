/************************************************
*                     File.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.11.2017                  *
*************************************************/
#pragma once

#include <cstdio>
#include <string>

#include <System/Assert.h>

namespace Columbus
{

	class C_File
	{
	private:
		struct C_FileData;
		C_FileData* mData;
	public:
		C_File();
		C_File(C_File& aOther);
		C_File(std::string aFile, std::string aModes);

		C_File& operator=(C_File& aOther);
		C_File& operator<<(const char aChar);
		C_File& operator<<(const std::string aString);

		bool open(std::string aFile, std::string aModes);
		bool close();

		std::string getName() const;
		int getSize() const;

		bool eof() const;
		int getc() const;
		bool seekSet(long int aOffset) const;
		bool seekEnd(long int aOffset) const;
		bool seekCur(long int aOffset) const;
		int tell() const;
		bool flush() const;

		bool isOpened() const;

		size_t read(void* aData, size_t aSize, size_t aPacks) const;
		size_t write(const void* aData, size_t aSize, size_t aPacks) const;

		bool readBytes(void* aData, size_t aSize);
		bool readUint8(uint8_t* aData);
		bool readInt8(int8_t* aData);
		bool readUint16(uint16_t* aData);
		bool readInt16(int16_t* aData);
		bool readUint32(uint32_t* aData);
		bool readInt32(int32_t* aData);

		bool writeBytes(const void* aData, size_t aSize);
		bool writeUint8(const uint8_t* aData);
		bool writeInt8(const int8_t* aData);
		bool writeUint16(const uint16_t* aData);
		bool writeInt16(const int16_t* aData);
		bool writeUint32(const uint32_t* aData);
		bool writeInt32(const int32_t* aData);

		~C_File();
	};

}







