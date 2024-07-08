#include "AssetImport.h"
#include "Core/Core.h"
#include "Common/Image/Image.h"

#include <filesystem>

// third party
#include <Lib/tinygltf/tiny_gltf.h>

namespace Columbus::Assets
{

	// TODO: assimp for import and gltf for internal storage?
	// TODO: mesh importing path, mesh optimisation, lod generation

	void ImportLevel(const char* SourcePath, const char* ImportPath)
	{
		using ImageTableType = std::unordered_map<int, Image>;
		ImageTableType ImagesTable;

		const tinygltf::LoadImageDataFunction LoadImageFn = [](tinygltf::Image* Img, const int ImgId, std::string* err, std::string* warn,
			int req_width, int req_height, const unsigned char* bytes, int size, void* user_data) -> bool
		{
			ImageTableType& ImagesTable = *(ImageTableType*)user_data;
			Image& LoadImg = ImagesTable[ImgId];

			DataStream Stream = DataStream::CreateFromMemory((u8*)bytes, size);

			if (!LoadImg.LoadFromStream(Stream))
			{
				ImagesTable.erase(ImgId);
				return false;
			}

			Img->image.resize(1); // so that write callback will be called

			return true;
		};

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err, warn;

		loader.SetImageLoader(LoadImageFn, &ImagesTable);

		Timer GltfTimer;
		if (!loader.LoadASCIIFromFile(&model, &err, &warn, SourcePath))
		{
			DEBUGBREAK();
			Log::Fatal("Couldn't load scene, %s", SourcePath);
		}
		Log::Message("GLTF loaded, time: %0.2f s", GltfTimer.Elapsed());

		// TODO: pack ORM textures

		// generate mips
		{
			Timer MipTotalTimer;
			Log::Message("[ImportLevel] Generating mips for %i textures", ImagesTable.size());

			for (auto& Element : ImagesTable)
			{
				Timer MipTimer;

				Image ImageWithMips;
				ImageMips::GenerateImageMips(Element.second, ImageWithMips, {});
				Element.second = ImageWithMips;

				Log::Message("[ImportLevel] Generating mips for %s finished, took %.3f seconds", model.images[Element.first].name.c_str(), MipTimer.Elapsed());
			}

			Log::Message("[ImportLevel] Generating mips finished, took %.3f seconds in total", MipTotalTimer.Elapsed());
		}

		// compress images
		{
			Timer CompressTotalTimer;
			Log::Message("[ImportLevel] Compressing %i textures", ImagesTable.size());

			for (auto& Element : ImagesTable)
			{
				Timer CompressTimer;

				Image CompressImage;
				ImageCompression::CompressionParams Params
				{
					.Format = TextureFormat::DXT1
				};

				ImageCompression::CompressImage(Element.second, CompressImage, Params);
				Element.second = CompressImage;

				Log::Message("[ImportLevel] Compression of %s finished, took %.3f seconds", model.images[Element.first].name.c_str(), CompressTimer.Elapsed());
			}

			Log::Message("[ImportLevel] Compression finished, took %.3f seconds in total", CompressTotalTimer.Elapsed());
		}

		// rewire names and save images
		{
			Log::Message("[ImportLevel] Writing %i textures", ImagesTable.size());

			std::filesystem::path BaseImportPath = std::filesystem::path(ImportPath).parent_path();

			for (int i = 0; i < (int)model.images.size(); i++)
			{
				tinygltf::Image& modelImage = model.images[i];

				auto NamePath = std::filesystem::path(modelImage.name);
				if (NamePath.has_extension())
				{
					NamePath.replace_extension("dds");
					modelImage.name = NamePath.string();
				}

				modelImage.uri = modelImage.uri.erase(modelImage.uri.find_last_of('.')) + ".dds";

				// TOOD: if URI leads to a weird path, it will all be weird
				// TODO: so need some separate asset database to handle these situations?
				std::filesystem::path SavePath = BaseImportPath / modelImage.uri;
				std::filesystem::create_directories(SavePath.parent_path());

				DataStream Stream = DataStream::CreateFromFile(SavePath.string().c_str(), "wb");
				ImageUtils::ImageSaveToStreamDDS(Stream, ImagesTable[i]);
			}

			Log::Message("[ImportLevel] Compressing of textures finished");
		}

		const tinygltf::WriteImageDataFunction WriteImageFn = [](const std::string* basepath,
			const std::string* filename,
			const tinygltf::Image* image, bool embedImages,
			const tinygltf::URICallbacks* uri_cb,
			std::string* out_uri,
			void* user_pointer)
		{
			*out_uri = image->uri;
			return true;
		};

		tinygltf::TinyGLTF saver;
		saver.SetImageWriter(WriteImageFn, nullptr);
		saver.WriteGltfSceneToFile(&model, ImportPath, false, false, true, false);
	}

}