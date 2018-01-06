/************************************************
*              	   ImageJPG.cpp                 *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   06.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>
#include <System/File.h>

namespace Columbus
{

	bool ImageIsJPG(std::string aFile)
	{
		C_File file(aFile, "rb");
		if (!file.isOpened()) return false;

		uint8_t magic[4];
		file.seekCur(6);
		if (!file.readBytes(magic, sizeof(magic))) return false;
		file.close();
		
		if (magic[0] == 'J' &&
			magic[1] == 'F' &&
			magic[2] == 'I' &&
			magic[3] == 'F') return true;
		else return false;
	}

}

