/************************************************
*              	   Compress.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   05.01.2018                  *
*************************************************/
#pragma once
#include <System/File.h>

namespace Columbus
{

	size_t CompressRLE(const uint8_t* aIn, uint8_t** aOut, const size_t aSize);
	void DecompressRLE(const uint8_t* aIn, uint8_t* aOut, const size_t aSize);

}





