#include <Common/Image/Image.h>
#include <System/File.h>
//#include <jpeglib.h>
//#include <setjmp.h>
#include <cstring>
#include <stb_image.h>
#include <Common/Image/ImageInternalCommon.h>

namespace Columbus::ImageUtils
{

	bool ImageCheckFormatFromStreamJPG(DataStream& Stream)
	{
		u8 magic[3];
		Stream.ReadBytes(magic, sizeof(magic));
		Stream.SeekSet(0); // rewind

		if (magic[0] == 0xFF &&
			magic[1] == 0xD8 &&
			magic[2] == 0xFF)
		{
			return true;
		}

		return false;
	}

	bool ImageLoadFromStreamJPG(DataStream& Stream, u32& OutWidth, u32& OutHeight, u32& OutMips, TextureFormat& OutFormat, ImageType& OutType, u8*& OutData)
	{
		stbi_io_callbacks Callbacks = StreamToStbCallbacks(Stream);

		int x, y, chans;
		OutData = (u8*)stbi_load_from_callbacks(&Callbacks, &Stream, &x, &y, &chans, 0);
		OutWidth = x;
		OutHeight = y;
		OutMips = 1;
		OutType = ImageType::Image2D;

		if (chans == 1) OutFormat = TextureFormat::R8;
		if (chans == 3) OutFormat = TextureFormat::RGB8;
		if (chans == 4) OutFormat = TextureFormat::RGBA8;

		return OutData != nullptr;

		/*struct jpeg_decompress_struct cinfo;

		struct jpeg_error_mgr pub;
		jmp_buf setjmp_buffer;

		FILE* file = fopen(FileName, "rb");
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
		//case JCS_EXT_RGBA:  bpp = 4; break;
		default: bpp = 0; break;
		}

		OutWidth = cinfo.image_width;
		OutHeight = cinfo.image_height;
		OutSize = cinfo.image_width * cinfo.image_height * bpp;
		switch (bpp)
		{
		case 1:  OutFormat = TextureFormat::R8;      break;
		case 3:  OutFormat = TextureFormat::RGB8;    break;
		case 4:  OutFormat = TextureFormat::RGBA8;   break;
		default: OutFormat = TextureFormat::Unknown; break;
		}

		jpeg_start_decompress(&cinfo);

		row_stride = cinfo.output_width * cinfo.output_components;
		buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

		uint8* data = new uint8[cinfo.image_width * cinfo.image_height * bpp];
		uint64 counter = 0;

		while (cinfo.output_scanline < cinfo.output_height)
		{
			jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(data + counter, buffer[0], row_stride);
			counter += row_stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		fclose(file);

		return data;*/
	}

	bool ImageSaveToFileJPG(const char* FileName, u32 Width, u32 Height, TextureFormat Format, u8* Data, u32 Quality)
	{
		return false;
		/*if (Data == nullptr) return false;

		struct jpeg_compress_struct cinfo;
		struct jpeg_error_mgr jerr;
		
		FILE* file = fopen(FileName, "wb");
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

		return true;*/
	}

}
