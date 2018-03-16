/************************************************
*              	    Image.cpp                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   02.01.2018                  *
*************************************************/
#include <Common/Image/Image.h>

namespace Columbus
{

	ImageFormat ImageGetFormat(std::string aFile)
	{
		if (ImageIsBMP(aFile)) return E_IMAGE_FORMAT_BMP;
		if (ImageIsPNG(aFile)) return E_IMAGE_FORMAT_PNG;
		if (ImageIsTIF(aFile)) return E_IMAGE_FORMAT_TIF;
		if (ImageIsJPG(aFile)) return E_IMAGE_FORMAT_JPG;
		if (ImageIsTGA(aFile)) return E_IMAGE_FORMAT_TGA;

		return E_IMAGE_FORMAT_UNKNOWN;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool ImageBGR2RGB(uint8_t* aData, size_t aSize)
	{
		if (aData == nullptr) return false;

		uint8_t bgr[3];
		for (size_t i = 0; i < aSize; i += 3)
		{
			bgr[0] = aData[i + 0];
			bgr[1] = aData[i + 1];
			bgr[2] = aData[i + 2];

			aData[i + 0] = bgr[2];
			aData[i + 1] = bgr[1];
			aData[i + 2] = bgr[0];
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageBGRA2RGBA(uint8_t* aData, size_t aSize)
	{
		if (aData == nullptr) return false;

		uint8_t bgr[3];
		for (size_t i = 0; i < aSize; i += 4)
		{
			bgr[0] = aData[i + 0];
			bgr[1] = aData[i + 1];
			bgr[2] = aData[i + 2];

			aData[i + 0] = bgr[2];
			aData[i + 1] = bgr[1];
			aData[i + 2] = bgr[0];
			aData[i + 3] = aData[i + 3];
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageABGR2RGBA(uint8_t* aData, size_t aSize)
	{
		if (aData == nullptr) return false;

		uint8_t abgr[4];
		for (size_t i = 0; i < aSize; i += 4)
		{
			abgr[0] = aData[i + 0];
			abgr[1] = aData[i + 1];
			abgr[2] = aData[i + 2];
			abgr[3] = aData[i + 3];

			aData[i + 0] = abgr[3];
			aData[i + 1] = abgr[2];
			aData[i + 2] = abgr[1];
			aData[i + 3] = abgr[0];
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageRGB2BGR(uint8_t* aData, size_t aSize)
	{
		return ImageBGR2RGB(aData, aSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageRGBA2BGRA(uint8_t* aData, size_t aSize)
	{
		return ImageBGRA2RGBA(aData, aSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageFlipX(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP)
	{
		if (aData == nullptr) return false;

		const size_t stride = aWidth * aBPP;
		uint8_t* row;
		uint8_t* pixel = (uint8_t*)malloc(aBPP);

		for (size_t i = 0; i < aHeight; i++)
		{
			row = &aData[i * stride];

			for (size_t j = 0; j < aWidth / 2; j++)
			{
				memcpy(pixel, &row[j * aBPP], aBPP);
				memcpy(&row[j * aBPP], &row[(aWidth - j) * aBPP], aBPP);
				memcpy(&row[(aWidth - j) * aBPP], pixel, aBPP);
			}
		}

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageFlipY(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP)
	{
		if (aData == nullptr) return false;

		const size_t stride = aWidth * aBPP;
		uint8_t* row = (uint8_t*)malloc(stride);

		for (size_t i = 0; i < aHeight / 2; i++)
		{
			memcpy(row, &aData[i * stride], stride);
			memcpy(&aData[i * stride], &aData[(aHeight - i - 1) * stride], stride);
			memcpy(&aData[(aHeight - i - 1) * stride], row, stride);
		}
		free(row);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageFlipXY(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP)
	{
		if (aData == nullptr) return false;

		const size_t size = aWidth * aHeight;
		uint8_t* pixel = new uint8_t[aBPP];

		for (size_t i = 0; i < size / 2; i++)
		{
			memcpy(pixel, &aData[i * aBPP], aBPP);
			memcpy(&aData[i * aBPP], &aData[(size - i) * aBPP], aBPP);
			memcpy(&aData[(size - i) * aBPP], pixel, aBPP);
		}
		free(pixel);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned char* ImageLoad(const std::string aFile, unsigned int& aWidth, unsigned int& aHeight, unsigned int& aBPP)
	{
		switch (ImageGetFormat(aFile))
		{
		case E_IMAGE_FORMAT_BMP: return ImageLoadBMP(aFile, aWidth, aHeight, aBPP); break;
		case E_IMAGE_FORMAT_PNG: return ImageLoadPNG(aFile, aWidth, aHeight, aBPP); break;
		case E_IMAGE_FORMAT_TIF: return ImageLoadTIF(aFile, aWidth, aHeight, aBPP); break;
		case E_IMAGE_FORMAT_JPG: return ImageLoadJPG(aFile, aWidth, aHeight, aBPP); break;
		case E_IMAGE_FORMAT_TGA: return ImageLoadTGA(aFile, aWidth, aHeight, aBPP); break;
		case E_IMAGE_FORMAT_UNKNOWN: return nullptr; break;
		default: return nullptr; break;
		}

		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageSave(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, 
		const unsigned int aBPP, const unsigned char* aData, const unsigned int aFormat, const unsigned int aQuality)
	{
		switch (aFormat)
		{
		case E_IMAGE_FORMAT_BMP:
			return ImageSaveBMP(aFile, aWidth, aHeight, aBPP, aData);
			break;
		case E_IMAGE_FORMAT_TGA:
			return ImageSaveTGA(aFile, aWidth, aHeight, aBPP, aData);
			break;
		case E_IMAGE_FORMAT_PNG:
			return ImageSavePNG(aFile, aWidth, aHeight, aBPP, aData);
			break;
		case E_IMAGE_FORMAT_TIF:
			return ImageSaveTIF(aFile, aWidth, aHeight, aBPP, aData);
			break;
		case E_IMAGE_FORMAT_JPG:
			return ImageSaveJPG(aFile, aWidth, aHeight, aBPP, aData, aQuality);
			break;
		}

		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Image::Image() :
		mWidth(0),
		mHeight(0),
		mBPP(0),
		mExist(false),
		mData(nullptr)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	Image::Image(std::string aFile, int aFlags) :
		mWidth(0),
		mHeight(0),
		mBPP(0),
		mExist(false),
		mData(nullptr)
	{
		load(aFile, aFlags);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Image::load(std::string aFile, int aFlags)
	{
		freeData();

		mData = ImageLoad(aFile, mWidth, mHeight, mBPP);
		if (mData == nullptr) return false;
		else
		{
			mFilename = aFile;
			mExist = true;

			switch (aFlags)
			{
			case E_IMAGE_LOAD_FLIP_X: flipX(); break;
			case E_IMAGE_LOAD_FLIP_Y: flipY(); break;
			case E_IMAGE_LOAD_FLIP_XY: flipXY(); break;
			}

			return true;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Image::save(std::string aFile, int aFormat, size_t aQuality) const
	{
		if (!isExist()) return false;
		return ImageSave(aFile, mWidth, mHeight, mBPP, mData, aFormat, aQuality);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Image::isExist() const
	{
		return mExist;
	}
	//////////////////////////////////////////////////////////////////////////////
	void Image::freeData()
	{
		if (mExist == false) return;
		mWidth = 0;
		mHeight = 0;
		mBPP = 0;
		mExist = false;
		if (mData == nullptr) return;
		free(mData);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	bool Image::flipX()
	{
		if (!isExist()) return false;
		return ImageFlipX(mData, mWidth, mHeight, mBPP);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Image::flipY()
	{
		if (!isExist()) return false;
		return ImageFlipY(mData, mWidth, mHeight, mBPP);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool Image::flipXY()
	{
		if (!isExist()) return false;
		return ImageFlipXY(mData, mWidth, mHeight, mBPP);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned int Image::getWidth() const
	{
		return mWidth;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int Image::getHeight() const
	{
		return mHeight;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int Image::getBPP() const
	{
		return mBPP;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned char* Image::getData() const
	{
		return mData;
	}
	//////////////////////////////////////////////////////////////////////////////
	std::string Image::getFilename() const
	{
		return mFilename;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	Image::~Image()
	{
		freeData();
	}
}
