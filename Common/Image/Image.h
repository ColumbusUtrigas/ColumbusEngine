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
#include <string>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	bool ImageIsBMP(std::string aFile); //Check file magic
	bool ImageIsTGA(std::string aFile); //Check file extension (*.tga, *.vda, *.icb, *.vst)
	bool ImageIsPNG(std::string aFile); //Check file magic
	bool ImageIsJPG(std::string aFile); //Check file magic
	bool ImageIsTIF(std::string aFile); //Check file magic

	unsigned char* ImageLoadBMP(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP);
	unsigned char* ImageLoadTGA(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP);
	unsigned char* ImageLoadPNG(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP);
	unsigned char* ImageLoadTIF(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP);

	bool ImageSaveBMP(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData);
	bool ImageSaveTGA(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData);
	bool ImageSavePNG(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData);
	bool ImageSaveTIF(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData);

	unsigned char* ImageLoad(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP);
	bool ImageSave(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData, const unsigned int aFormat, const unsigned int aQuality = 100);

	bool ImageBGR2RGB(uint8_t* aData, size_t aSize);
	bool ImageBGRA2RGBA(uint8_t* aData, size_t aSize);
	bool ImageABGR2RGBA(uint8_t* aData, size_t aSize);
	bool ImageRGB2BGR(uint8_t* aData, size_t aSize);
	bool ImageRGBA2BGRA(uint8_t* aData, size_t aSize);

	bool ImageFlipX(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP);
	bool ImageFlipY(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP);
	bool ImageFlipXY(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP);

	enum E_IMAGE_LOAD
	{
		E_IMAGE_NONE,
		E_IMAGE_FLIP_X,
		E_IMAGE_FLIP_Y
	};

	enum E_IMAGE_SAVE
	{
		E_IMAGE_SAVE_FORMAT_BMP,
		E_IMAGE_SAVE_FORMAT_TGA,
		E_IMAGE_SAVE_FORMAT_PNG
	};

	class C_Image
	{
	private:
		unsigned int mWidth = 0;         //Width of the image
		unsigned int mHeight = 0;        //Height of the image
		unsigned int mBPP = 0;           //Byte (!) depth of the image: 3, 4
		unsigned char* mData = nullptr;  //Pixel data
		bool mExist = false;             //Is image exist

		std::string mFilename;
	public:
		C_Image();
		C_Image(const std::string aFile, const unsigned int aFlags = E_IMAGE_NONE);

		bool load(const std::string aFile, const unsigned int aFlags = E_IMAGE_NONE);
		bool save(const std::string aFile, const unsigned int aFlags) const;
		bool isExist() const;
		void freeData(); //This method checks image existance

		bool flipX();
		bool flipY();

		unsigned int getWidth() const;
		unsigned int getHeight() const;
		unsigned int getBPP() const;
		unsigned char* getData() const;
		std::string getFilename() const;

		~C_Image();
	};

}



