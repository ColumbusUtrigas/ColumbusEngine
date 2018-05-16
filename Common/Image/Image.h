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

	enum class ImageLoading
	{
		None,
		FlipX,
		FlipY,
		FlipXY
	};

	enum class ImageFormat
	{
		BMP,
		DDS,
		PNG,
		TIF,
		JPG,
		TGA,
		Unknown
	};

	enum class TextureFormat
	{
		RGB,
		RGBA,
		S3TC_A1,
		S3TC_A4,
		S3TC_A8,
		Unknown
	};

	ImageFormat ImageGetFormat(std::string FileName);
	uint32 GetBPPFromFormat(TextureFormat Format);

	bool ImageIsBMP(std::string FileName); //Check file magic
	bool ImageIsDDS(std::string FileName); //Check file magic
	bool ImageIsDDSMemory(const uint8* Data, uint64 Size);
	bool ImageIsTGA(std::string FileName); //Check file extension (*.tga, *.vda, *.icb, *.vst)
	bool ImageIsPNG(std::string FileName); //Check file magic
	bool ImageIsTIF(std::string FileName); //Check file magic
	bool ImageIsJPG(std::string FileName); //Check file magic

	uint8* ImageLoadBMP(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat);
	uint8* ImageLoadDDS(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, uint32& OutMipMaps, TextureFormat& OutFormat);
	uint8* ImageLoadDDSMemory(const uint8* Data, uint64 Size, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, uint32& OutMipMaps, TextureFormat& OutFormat);
	uint8* ImageLoadTGA(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat);
	uint8* ImageLoadPNG(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat);
	uint8* ImageLoadTIF(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat);
	uint8* ImageLoadJPG(std::string FileName, uint32& OutWidth, uint32& OutHeight, TextureFormat& OutFormat);

	bool ImageSaveBMP(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTGA(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSavePNG(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveTIF(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data);
	bool ImageSaveJPG(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data, uint32 Quality = 100);

	uint8* ImageLoad(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, uint32& OutMipMaps, TextureFormat& OutFormat);
	bool ImageSave(std::string FileName, uint32 Width, uint32 Height, TextureFormat BPP, uint8* Data, ImageFormat Format, uint32 Quality = 100);

	bool ImageBGR2RGB(uint8* Data, uint64 Size);
	bool ImageBGRA2RGBA(uint8* Data, uint64 Size);
	bool ImageABGR2RGBA(uint8* Data, uint64 Size);
	bool ImageRGB2BGR(uint8* Data, uint64 Size);
	bool ImageRGBA2BGRA(uint8* Data, uint64 Size);

	bool ImageFlipX(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);
	bool ImageFlipY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);
	bool ImageFlipXY(uint8* Data, uint32 Width, uint32 Height, uint32 BPP);

	class Image
	{
	private:
		uint32 Width = 0;         //Width of the image
		uint32 Height = 0;        //Height of the image
		uint64 Size = 0;
		uint32 MipMaps = 0;
		TextureFormat Format = TextureFormat::RGBA;
		uint8* Data = nullptr;    //Pixel data
		bool Exist = false;       //Is image exist

		std::string FileName;
	public:
		Image();

		bool Load(std::string InFilename, ImageLoading Flags = ImageLoading::None);
		bool Save(std::string InFilename, ImageFormat Format, size_t Quality = 100) const;
		bool IsExist() const;
		void FreeData();

		bool FlipX();
		bool FlipY();
		bool FlipXY();

		uint32 GetWidth() const;
		uint32 GetHeight() const;
		uint64 GetSize() const;
		TextureFormat GetFormat() const;
		uint8* GetData() const;
		std::string GetFileName() const;

		~Image();
	};

}



