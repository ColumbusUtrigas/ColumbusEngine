#include <Common/Image/Image.h>
#include <Common/Image/JPG/ImageJPG.h>
#include <Core/Memory.h>
#include <System/File.h>
#include <jpeglib.h>
#include <setjmp.h>

namespace Columbus
{

	static uint8* ImageLoadJPG(std::string FileName, uint32& OutWidth, uint32& OutHeight, uint64& OutSize, TextureFormat& OutFormat)
	{
		struct jpeg_decompress_struct cinfo;

		struct jpeg_error_mgr pub;
		jmp_buf setjmp_buffer;

		FILE* file = fopen(FileName.c_str(), "rb");
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

		int bpp;
		switch (cinfo.out_color_space)
		{
			case JCS_GRAYSCALE: bpp = 1; break;
			case JCS_RGB:       bpp = 3; break;
			case JCS_EXT_RGBA:  bpp = 4; break;
		}

		OutWidth = cinfo.image_width;
		OutHeight = cinfo.image_height;
		OutSize = cinfo.image_width * cinfo.image_height * bpp;
		switch (bpp)
		{
			case 1: OutFormat = TextureFormat::R8;    break;
			case 3: OutFormat = TextureFormat::RGB8;  break;
			case 4: OutFormat = TextureFormat::RGBA8; break;
		}

		jpeg_start_decompress(&cinfo);

		row_stride = cinfo.output_width * cinfo.output_components;
		buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

		//uint8* data = (uint8*)Memory::Malloc(cinfo.image_width * cinfo.image_height * bpp);
		uint8* data = new uint8[cinfo.image_width * cinfo.image_height * bpp];
		uint64 counter = 0;
		uint64 maxsize = row_stride * cinfo.image_height;

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			Memory::Memcpy(data + (maxsize - counter - row_stride), buffer[0], row_stride);
			counter += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		fclose(file);

		return data;
	}

	bool ImageSaveJPG(std::string FileName, uint32 Width, uint32 Height, TextureFormat Format, uint8* Data, uint32 Quality)
	{
		if (Data == nullptr) return false;

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		
		FILE* file = fopen(FileName.c_str(), "wb");
		if (file == nullptr) return false;

		JSAMPROW row_pointer[1];
		size_t row_stride;

		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_compress(&cinfo);

		jpeg_stdio_dest(&cinfo, file);

		uint32 BPP = GetBPPFromFormat(Format);

		cinfo.image_width = Width;
		cinfo.image_height = Height;
		cinfo.input_components = BPP;
		cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, Quality, TRUE);

		jpeg_start_compress(&cinfo, TRUE);

		row_stride = Width * BPP;

		uint64 maxsize = row_stride * Height;

		while (cinfo.next_scanline < cinfo.image_height)
		{
			row_pointer[0] = (JSAMPROW)&Data[maxsize - cinfo.next_scanline * row_stride - row_stride];
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		fclose(file);
		jpeg_destroy_compress(&cinfo);

		return true;
	}

	bool ImageLoaderJPG::IsJPG(std::string FileName)
	{
		File JPGImageFile(FileName, "rb");
		if (!JPGImageFile.IsOpened()) return false;

		uint8_t magic[3];
		if (!JPGImageFile.ReadBytes(magic, sizeof(magic))) return false;
		JPGImageFile.Close();

		if (magic[0] == 0xFF &&
		    magic[1] == 0xD8 &&
		    magic[2] == 0xFF)
		{
		    return true;
		}
		else
		{
			return false;
		}
	}

	bool ImageLoaderJPG::Load(std::string FileName)
	{
		Free();
		uint64 Size = 0;

		Data = ImageLoadJPG(FileName, Width, Height, Size, Format);

		return (Data != nullptr);
	}

	void ImageLoaderJPG::Free()
	{
		delete[] Data;
		Width = 0;
		Height = 0;
		Mipmaps = 0;
		Format = TextureFormat::RGBA8;
	}

}

