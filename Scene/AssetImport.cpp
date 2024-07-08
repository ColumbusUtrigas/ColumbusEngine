#include "AssetImport.h"
#include "Core/Core.h"
#include "Common/Image/Image.h"

// third party
#include <Lib/tinygltf/tiny_gltf.h>

namespace Columbus::Assets
{

	// TODO: assimp for import and gltf for internal storage?
	// TODO: mesh importing path, mesh optimisation, lod generation

	void ImportLevel(const char* SourcePath, const char* ImportPath)
	{
		const tinygltf::LoadImageDataFunction LoadImageFn = [](tinygltf::Image* Img, const int ImgId, std::string* err, std::string* warn,
			int req_width, int req_height, const unsigned char* bytes, int size, void* user_data) -> bool
		{
			u32 W, H, D, Mips;
			TextureFormat Format;
			ImageType Type;

			DataStream Stream = DataStream::CreateFromMemory((u8*)bytes, size);

			u8* Data = nullptr;
			if (!ImageUtils::ImageLoadFromStream(Stream, W, H, D, Mips, Format, Type, Data))
			{
				delete[] Data;
				return false;
			}

			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);

			u64 Size = size_t(W * H) * size_t(FormatInfo.BitsPerPixel) / 8;

			Img->width = W;
			Img->height = H;
			Img->component = FormatInfo.NumChannels;
			Img->bits = FormatInfo.BitsPerPixel;
			Img->pixel_type = (int)Format; // our small hack here
			Img->image.resize(Size);
			memcpy(Img->image.data(), Data, Size);
			delete[] Data;
			return true;
		};

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err, warn;

		loader.SetImageLoader(LoadImageFn, nullptr);

		Timer GltfTimer;
		if (!loader.LoadASCIIFromFile(&model, &err, &warn, SourcePath))
		{
			Log::Fatal("Couldn't load scene, %s", SourcePath);
		}
		Log::Message("GLTF loaded, time: %0.2f s", GltfTimer.Elapsed());

		// TODO: pack ORM textures
		// TODO: compress images

		// TODO: think about writing images properly, perhaps URI callback will be useful here with a modified texture ID
		const tinygltf::WriteImageDataFunction WriteImageFn = [](const std::string* basepath,
			const std::string* filename,
			const tinygltf::Image* image, bool embedImages,
			const tinygltf::URICallbacks* uri_cb,
			std::string* out_uri,
			void* user_pointer)
		{
			return true;
		};

		tinygltf::TinyGLTF saver;
		saver.SetImageWriter(WriteImageFn, nullptr);
		bool result = saver.WriteGltfSceneToFile(&model, ImportPath, true, true, true, false);
		int asd = 123;
	}

}