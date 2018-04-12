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

	ImageFormat ImageGetFormat(std::string FileName)
	{
		if (ImageIsBMP(FileName)) return E_IMAGE_FORMAT_BMP;
		if (ImageIsPNG(FileName)) return E_IMAGE_FORMAT_PNG;
		if (ImageIsTIF(FileName)) return E_IMAGE_FORMAT_TIF;
		if (ImageIsJPG(FileName)) return E_IMAGE_FORMAT_JPG;
		if (ImageIsTGA(FileName)) return E_IMAGE_FORMAT_TGA;

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
	uint8* ImageLoad(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint32& OutBPP)
	{
		switch (ImageGetFormat(FileName))
		{
		case E_IMAGE_FORMAT_BMP: return ImageLoadBMP(FileName, OutWidth, OutHeight, OutBPP); break;
		case E_IMAGE_FORMAT_PNG: return ImageLoadPNG(FileName, OutWidth, OutHeight, OutBPP); break;
		case E_IMAGE_FORMAT_TIF: return ImageLoadTIF(FileName, OutWidth, OutHeight, OutBPP); break;
		case E_IMAGE_FORMAT_JPG: return ImageLoadJPG(FileName, OutWidth, OutHeight, OutBPP); break;
		case E_IMAGE_FORMAT_TGA: return ImageLoadTGA(FileName, OutWidth, OutHeight, OutBPP); break;
		case E_IMAGE_FORMAT_UNKNOWN: return nullptr; break;
		default: return nullptr; break;
		}

		return nullptr;
	}
	//////////////////////////////////////////////////////////////////////////////
	bool ImageSave(std::string FileName, uint32 Width, uint32 Height, uint32 BPP, uint8* Data, int Format, uint32 Quality)
	{
		switch (Format)
		{
		case E_IMAGE_FORMAT_BMP:
			return ImageSaveBMP(FileName, Width, Height, BPP, Data);
			break;
		case E_IMAGE_FORMAT_TGA:
			return ImageSaveTGA(FileName, Width, Height, BPP, Data);
			break;
		case E_IMAGE_FORMAT_PNG:
			return ImageSavePNG(FileName, Width, Height, BPP, Data);
			break;
		case E_IMAGE_FORMAT_TIF:
			return ImageSaveTIF(FileName, Width, Height, BPP, Data);
			break;
		case E_IMAGE_FORMAT_JPG:
			return ImageSaveJPG(FileName, Width, Height, BPP, Data, Quality);
			break;
		}

		return false;
	}
	/*
	* Image class
	*/
	Image::Image() :
		Width(0),
		Height(0),
		BPP(0),
		Exist(false),
		Data(nullptr)
	{ }
	/*
	* Load image from file
	* @param std::string InFileName: Name of image file to load
	* @param int Flags: Loading flags
	*/
	bool Image::load(std::string InFileName, int Flags)
	{
		freeData();

		Data = ImageLoad(InFileName, Width, Height, BPP);
		if (Data == nullptr) return false;

		else
		{
			FileName = InFileName;
			Exist = true;

			switch (Flags)
			{
			case E_IMAGE_LOAD_FLIP_X: flipX(); break;
			case E_IMAGE_LOAD_FLIP_Y: flipY(); break;
			case E_IMAGE_LOAD_FLIP_XY: flipXY(); break;
			}

			return true;
		}
	}
	/*
	* Save image to file
	* @param std::string InFileName: Name of image file to save
	* @param Format: Image format
	* @param Quality: Compression level
	*/
	bool Image::save(std::string InFileName, int Format, size_t Quality) const
	{
		if (!isExist()) return false;
		return ImageSave(InFileName, Width, Height, BPP, Data, Format, Quality);
	}
	/*
	* Checks if image is exist in memory
	*/
	bool Image::isExist() const
	{
		return Exist;
	}
	/*
	* Frees image data
	*/
	void Image::freeData()
	{
		if (Exist == false) return;
		Width = 0;
		Height = 0;
		BPP = 0;
		Exist = false;

		if (Data != nullptr)
		{
			free(Data);
		}
	}
	/*
	* Horizontal image flipping
	*/
	bool Image::flipX()
	{
		if (isExist())
		{
			return ImageFlipX(Data, Width, Height, BPP);
		}

		return false;
	}
	/*
	* Vertical image flipping
	*/
	bool Image::flipY()
	{
		if (isExist())
		{
			return ImageFlipY(Data, Width, Height, BPP);
		}

		return false;
	}
	/*
	* Diagonal image flipping
	*/
	bool Image::flipXY()
	{
		if (!isExist()) return false;
		return ImageFlipXY(Data, Width, Height, BPP);
	}
	
	uint32 Image::getWidth() const
	{
		return Width;
	}
	
	uint32 Image::getHeight() const
	{
		return Height;
	}
	
	uint32 Image::getBPP() const
	{
		return BPP;
	}
	
	uint8* Image::getData() const
	{
		return Data;
	}
	
	std::string Image::getFileName() const
	{
		return FileName;
	}
	
	Image::~Image()
	{
		freeData();
	}
}






