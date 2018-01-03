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

	bool ImageBGR2RGB(uint8_t* aData, size_t aSize)
	{
		if (aData == nullptr) return false;

		uint8_t bgr[3];
		for (int i = 0; i < aSize; i += 3)
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
		if (aData == nullptr) return 0;

		uint8_t bgr[3];
		for (int i = 0; i < aSize; i += 4)
		{
			bgr[0] = aData[i + 0];
			bgr[1] = aData[i + 1];
			bgr[2] = aData[i + 2];

			aData[i + 0] = bgr[2];
			aData[i + 1] = bgr[1];
			aData[i + 2] = bgr[0];
			aData[i + 3] = aData[i + 3];
		}

		return 1;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageRGB2BGR(uint8_t* aData, size_t aSize)
	{
		return ImageBGR2RGB(aData, aSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageRGBA2BGRA(uint8_t* aData, size_t aSize)
	{
		return ImageBGR2RGB(aData, aSize);
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageFlipX(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP)
	{
		if (aData == nullptr) return false;

		const size_t stride = aWidth * aBPP;
		uint8_t* row = (uint8_t*)malloc(stride);

		for (size_t i = 0; i < aHeight; i++)
		{
			memcpy(row, &aData[stride * i], stride);
			std::reverse(row, &row[stride - 1]);
			memcpy(&aData[stride * i], row, stride);
		}
		free(row);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageFlipY(uint8_t* aData, size_t aWidth, size_t aHeight, size_t aBPP)
	{
		if (aData == nullptr) return false;

		const size_t stride = aWidth * aBPP;
		uint8_t* row = (uint8_t*)malloc(stride);
		uint8_t* low = aData;
		uint8_t* high = &aData[(aHeight - 1) * stride];

		for (; low < high; low += stride, high -= stride)
		{
			memcpy(row, low, stride);
			memcpy(low, high, stride);
			memcpy(high, row, stride);
		}
		free(row);

		return true;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned char* ImageLoad(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP)
	{
		if (ImageIsBMP(aFile))
			return ImageLoadBMP(aFile, aWidth, aHeight, aBPP);

		return false;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Image::C_Image() :
		mWidth(0),
		mHeight(0),
		mBPP(0),
		mExist(false),
		mData(nullptr)
	{

	}
	//////////////////////////////////////////////////////////////////////////////
	C_Image::C_Image(const std::string aFile, const unsigned int aFlags) :
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
	bool C_Image::load(const std::string aFile, const unsigned int aFlags)
	{
		mData = ImageLoad(aFile, &mWidth, &mHeight, &mBPP);
		if (mData == nullptr) return false;
		else
		{
			mFilename = aFile;
			mExist = true;

			switch (aFlags)
			{
			case E_IMAGE_FLIP_X:
				flipX();
				break;
			case E_IMAGE_FLIP_Y:
				flipY();
				break;
			}

			return true;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Image::save(const std::string aFile, const unsigned int aFormat) const
	{
		switch (aFormat)
		{
		case E_IMAGE_SAVE_FORMAT_BMP:
			return ImageSaveBMP(aFile, mWidth, mHeight, mBPP, mData);
			break;
		}
	}
	//////////////////////////////////////////////////////////////////////////////
	bool C_Image::isExist() const
	{
		return mExist;
	}
	//////////////////////////////////////////////////////////////////////////////
	void C_Image::freeData()
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
	bool C_Image::flipX()
	{
		if (!isExist()) return false;
		return ImageFlipX(mData, mWidth, mHeight, mBPP);
	}
	bool C_Image::flipY()
	{
		if (!isExist()) return false;
		return ImageFlipY(mData, mWidth, mHeight, mBPP);
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_Image::getWidth() const
	{
		return mWidth;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_Image::getHeight() const
	{
		return mHeight;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned int C_Image::getBPP() const
	{
		return mBPP;
	}
	//////////////////////////////////////////////////////////////////////////////
	unsigned char* C_Image::getData() const
	{
		return mData;
	}
	//////////////////////////////////////////////////////////////////////////////
	std::string C_Image::getFilename() const
	{
		return mFilename;
	}
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	C_Image::~C_Image()
	{
		freeData();
	}
}
