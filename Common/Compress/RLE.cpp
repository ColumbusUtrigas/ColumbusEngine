/************************************************
*              	      RLE.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.01.2018                  *
*************************************************/
#include <Common/Compress/Compress.h>
#include <System/Assert.h>
#include <System/File.h>

namespace Columbus
{

	size_t CompressRLE(const uint8_t* aIn, uint8_t** aOut, const size_t aSize)
	{
		COLUMBUS_ASSERT_MESSAGE(aIn, "CompressRLE(): invalid input data");

		uint8_t prevSym;
		uint8_t sym;
		size_t count = 1;
		size_t size = 0;
		uint8_t* buffer = (uint8_t*)malloc(aSize);
		size_t bufCounter = 0;

		for (size_t i = 0; i < aSize; i++)
		{
			sym = aIn[i];
			if (sym != prevSym)
			{
				buffer[bufCounter++] = static_cast<uint8_t>(count);
				buffer[bufCounter++] = aIn[i];
				size += 2;
				count = 0;
				prevSym = sym;
			}
			count++;
		}
		uint8_t* buf = (uint8_t*)malloc(size);
		memcpy(buf, buffer, size);
		free(buffer);
		*aOut = buf;

		return size;
	}

	void DecompressRLE(const uint8_t* aIn, uint8_t* aOut, const size_t aSize)
	{
		COLUMBUS_ASSERT_MESSAGE(aIn, "DecompressRLE(): invalid input data");
	}

}


