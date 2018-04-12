/************************************************
*              	     Image.h                    *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.01.2018                  *
*************************************************/
#pragma once

#include <System/Assert.h>
#include <System/System.h>
#include <Core/Types.h>
#include <string>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	enum ImageLoad
	{
		E_IMAGE_LOAD_NONE,
		E_IMAGE_LOAD_FLIP_X,
		E_IMAGE_LOAD_FLIP_Y,
		E_IMAGE_LOAD_FLIP_XY
	};

	enum ImageFormat
	{
		E_IMAGE_FORMAT_BMP,
		E_IMAGE_FORMAT_PNG,
		E_IMAGE_FORMAT_TIF,
		E_IMAGE_FORMAT_JPG,
		E_IMAGE_FORMAT_TGA,
		E_IMAGE_FORMAT_UNKNOWN
	};

	ImageFormat ImageGetFormat(std::string FileName);

	bool ImageIsBMP(std::string FileName); //Check file magic
	bool ImageIsTGA(std::string FileName); //Check file extension (*.tga, *.vda, *.icb, *.vst)
	bool ImageIsPNG(std::string FileName); //Check file magic
	bool ImageIsTIF(std::string FileName); //Check file magic
	bool ImageIsJPG(std::string FileName); //Check file magic

	uint8* ImageLoadBMP(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);
	uint8* ImageLoadTGA(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);
	uint8* ImageLoadPNG(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);
	uint8* ImageLoadTIF(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);
	uint8* ImageLoadJPG(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);

	bool ImageSaveBMP(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data);
	bool ImageSaveTGA(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data);
	bool ImageSavePNG(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data);
	bool ImageSaveTIF(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data);
	bool ImageSaveJPG(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data, uint32 Quality = 100);

	uint8* ImageLoad(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP);
	bool ImageSave(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data, int Format, uint32 Quality = 100);

	bool ImageBGR2RGB(uint8* Data, size_t Size);
	bool ImageBGRA2RGBA(uint8* Data, size_t Size);
	bool ImageABGR2RGBA(uint8* Data, size_t Size);
	bool ImageRGB2BGR(uint8* Data, size_t Size);
	bool ImageRGBA2BGRA(uint8* Data, size_t Size);

	bool ImageFlipX(uint8* Data, size_t Width, size_t Height, size_t BPP);
	bool ImageFlipY(uint8* Data, size_t Width, size_t Height, size_t BPP);
	bool ImageFlipXY(uint8* Data, size_t Width, size_t Height, size_t BPP);

	class Image
	{
	private:
		uint32 Width = 0;         //Width of the image
		uint32 Height = 0;        //Height of the image
		uint32 BPP = 0;           //Byte (!) depth of the image: 3, 4
		uint8* Data = nullptr;    //Pixel data
		bool Exist = false;       //Is image exist

		std::string FileName;
	public:
		Image();

		bool load(std::string InFilename, int Flags = E_IMAGE_LOAD_NONE);
		bool save(std::string InFilename, int Format, size_t Quality = 100) const;
		bool isExist() const;
		void freeData();

		bool flipX();
		bool flipY();
		bool flipXY();

		uint32 getWidth() const;
		uint32 getHeight() const;
		uint32 getBPP() const;
		uint8* getData() const;
		std::string getFileName() const;

		~Image();
	};

}



