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
#include <jpeglib.h>
#include <setjmp.h>

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

	unsigned char* ImageLoadJPG(const std::string aFile, unsigned int* aWidth, unsigned int* aHeight, unsigned int* aBPP)
	{
		COLUMBUS_ASSERT_MESSAGE(aWidth, "ImageLoadJPG(): invalid width")
		COLUMBUS_ASSERT_MESSAGE(aHeight, "ImageLoadJPG(): invalid height")
		COLUMBUS_ASSERT_MESSAGE(aBPP, "ImageLoadJPG(): invalid BPP")

		struct jpeg_decompress_struct cinfo;

		struct jpeg_error_mgr pub;
		jmp_buf setjmp_buffer;

		FILE* file = fopen(aFile.c_str(), "rb");
		JSAMPARRAY buffer;
		size_t row_stride;

		if (file == nullptr) return nullptr;

		cinfo.err = jpeg_std_error(&pub);
		
		if (setjmp(setjmp_buffer))
		{
			jpeg_destroy_decompress(&cinfo);
			fclose(file);
			return nullptr;
		}

		jpeg_create_decompress(&cinfo);

		jpeg_stdio_src(&cinfo, file);

		jpeg_read_header(&cinfo, TRUE);

		*aWidth = cinfo.image_width;
		*aHeight = cinfo.image_height;
		*aBPP = 3;

		jpeg_start_decompress(&cinfo);

		row_stride = cinfo.output_width * cinfo.output_components;
		buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

		uint8_t* data = (uint8_t*)malloc(cinfo.image_width * cinfo.image_height * 3);
		uint64_t counter = 0;

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(data + counter, buffer[0], row_stride);
			counter += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		fclose(file);

		return data;
	}

	bool ImageSaveJPG(const std::string aFile, const unsigned int aWidth, const unsigned int aHeight, const unsigned int aBPP, const unsigned char* aData, const unsigned int aQuality)
	{
		if (aData == nullptr) return false;

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		
		FILE* file = fopen(aFile.c_str(), "wb");;
		if (file == nullptr) return false;

		JSAMPROW row_pointer[1];
		size_t row_stride;

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		jpeg_stdio_dest(&cinfo, file);

		cinfo.image_width = aWidth;
		cinfo.image_height = aHeight;
		cinfo.input_components = aBPP;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, aQuality, TRUE);

		jpeg_start_compress(&cinfo, TRUE);

		row_stride = aWidth * aBPP;

		while (cinfo.next_scanline < cinfo.image_height)
		{
			row_pointer[0] = (JSAMPROW)&aData[cinfo.next_scanline * row_stride];
			(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		fclose(file);
		jpeg_destroy_compress(&cinfo);

		return true;
	}

}

