#include "Asset.h"
#include "System/Log.h"
#include "Serialisation.h"

#include <filesystem>
#include <fstream>

namespace Columbus
{

	void AssetSystem::ResolveRef(AssetRef<void>& Ref, const Reflection::Struct* Type)
	{
		void* AssetToUnload = nullptr;

		if (LoadedAssets.contains(Ref.Path))
		{
			AssetData& Data = LoadedAssets[Ref.Path];

			if (Ref.Asset == Data.Asset)
			{
				return; // already loaded
			}
			else
			{
				AssetToUnload = Ref.Asset; // unload previous asset if it was loaded
			}
		}

		Ref.Asset = LoadAssetRaw(Ref.Path, Type);
		UnloadAssetRaw(AssetToUnload);
	}

	void* AssetSystem::LoadAssetRaw(const std::string& Path, const Reflection::Struct* Type)
	{
		if (Path.empty())
		{
			Log::Warning("Asset reference %s is empty in %s", Type->Name);
			return nullptr;
		}

		void* Asset = nullptr;
		if (!LoadedAssets.contains(Path))
		{
			auto RealPath = (std::filesystem::path(SourceDataPath) / Path).string();
			if (AssetLoaderFunctions.contains(Type))
			{
				// load asset using the registered loader function
				Asset = AssetLoaderFunctions[Type](RealPath.c_str());
			}
			else
			{
				// otherwise assume it's a plain struct asset

				std::ifstream fs(RealPath);
				if (!fs.is_open())
				{
					Log::Error("Couldn't load asset, %s of type %s", Path, Type->Name);
					return nullptr;
				}

				nlohmann::json json;
				fs >> json;
				fs.close();

				std::string Guid = json["0_type_guid"];
				if (Guid != Type->Guid)
				{
					Log::Error("Asset type mismatch for %s: expected %s, got %s", Path, Type->Guid, Guid.c_str());
					return nullptr; // type mismatch
				}

				Asset = Type->Constructor();
				Reflection_DeserialiseStructJson((char*)Asset, Type, json);
			}

			LoadedAssets[Path] = AssetData{ Asset, Type, 0 };
			PathByAsset[Asset] = Path; // map asset to path for unloading

			ResolveStructAssetReferences(Type, Asset); // resolve asset references in the loaded asset
		}
		else
		{
			Asset = LoadedAssets[Path].Asset;

			if (Type != LoadedAssets[Path].Type)
			{
				Log::Error("Asset type mismatch for %s: expected %s, got %s", Path, LoadedAssets[Path].Type->Name, Type->Name);
				return nullptr; // type mismatch
			}
		}

		AssetData& Data = LoadedAssets[Path];
		Data.Asset = Asset;
		Data.RefCount++;

		return Asset;
	}

	void AssetSystem::UnloadAssetRaw(void* Asset)
	{
		if (!Asset)
			return;

		if (!PathByAsset.contains(Asset))
			return;

		auto Path = PathByAsset[Asset];
		const AssetData& Data = LoadedAssets[Path];

		LoadedAssets[Path].RefCount--;
		if (LoadedAssets[Path].RefCount <= 0)
		{
			if (AssetUnloaderFunctions.contains(Data.Type))
			{
				AssetUnloaderFunctions[Data.Type](Asset);
			}
			else
			{
				Data.Type->Destructor(Asset); // call destructor if no custom unloader is registered
			}

			LoadedAssets.erase(Path);
			PathByAsset.erase(Asset);
		}
	}

	void AssetSystem::ResolveStructAssetReferences(const Reflection::Struct* Struct, void* Object)
	{
		for (const auto& Field : Struct->Fields)
		{
			if (Field.Type == Reflection::FieldType::AssetRef)
			{
				AssetRef<void>* Ref = (AssetRef<void>*)((char*)Object + Field.Offset);
				ResolveRef(*Ref, Reflection::FindStructByGuid(Field.Typeguid));
			}
		}
	}

}
