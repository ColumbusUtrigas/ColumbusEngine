#include <Common/Image/HDR/ImageHDR.h>
#include <System/File.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_MALLOC(sz) ((void*)(new char[sz]))
#define STBI_REALLOC(p,newsz) realloc(p,newsz)
#define STBI_FREE(p) (delete[] ((char*)p))

#include <Lib/STB/stb_image.h>

namespace Columbus
{

	bool ImageLoaderHDR::IsHDR(const char* FileName)
	{
		File ImageFile(FileName, "rb");
		const char Cmp[] = "#?RADIANCE\n";
		char Magic[11];
		ImageFile.Read(Magic);
		return memcmp(Magic, Cmp, 11) == 0;
	}

	bool ImageLoaderHDR::Load(const char* FileName)
	{
		int x, y, comp;
		stbi_set_flip_vertically_on_load(true);
		Data = (uint8*)stbi_loadf(FileName, &x, &y, &comp, 0);
		Width = x;
		Height = y;
		Format = TextureFormat::RGB16F;
		ImageType = ImageLoader::Type::Image2D;

		return Data != nullptr;
	}

}


