#pragma once

#include "Core/Reflection.h"
#include "Core/Assert.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>

// system description:
// 
// AssetRef is a shared immutable resource, it can refernce any reflected type and types like textures, sounds, levels
// AssetRef helps to manage lifetime of an asset, its loading and unloading, and can be used in the editor UI
// 
// AssetSystem provides a central hub for resolving references and holding resources
// AssetSystem should not be used directly as much as possible, use AssetRef instead
// By default, AssetSystem will treat any asset as a serialised struct, but you can register custom loaders and unloaders for specific types (like textures and sounds)

namespace Columbus
{

	struct AssetSystem;

	template <typename T>
	struct AssetRef
	{
		std::string Path;
		T* Asset = nullptr;

		AssetRef()
		{
		}

		AssetRef(std::string_view P) : Path(P)
		{
		}

		AssetRef(const AssetRef<T>& Other)
		{
			*this = Other;
		}

		AssetRef<T>& operator=(const AssetRef<T>& Other)
		{
			if (Other.Asset)
			{
				Path = Other.Path;
				Resolve(); // increase refcount
			}
			else
			{
				Unload(); // decrease refcount
				Path = Other.Path;
			}

			return *this;
		}

		T* operator->()
		{
			return Asset;
		}

		operator bool() const
		{
			return IsValid();
		}

		bool IsValid() const
		{
			return Asset != nullptr;
		}

		~AssetRef()
		{
			Unload();
		}

		void Resolve()
		{
			AssetSystem::Get().ResolveRef(*this);
		}

		void Unload()
		{
			AssetSystem::Get().UnloadRef(*this);
		}

		const char* GetTypeGuid() const
		{
			return Reflection::FindTypeGuid<T>();
		}
	};


	// paths in the asset system are relative to the project data path
	struct AssetSystem
	{
		static AssetSystem& Get()
		{
			static AssetSystem Instance;
			return Instance;
		}

	public:
		// public asset manipulation API

		bool IsPathInSourceFolder(const std::string& Path) const;
		bool IsPathInBakedFolder(const std::string& Path) const;
		std::string MakePathRelativeToSourceFolder(const std::string& Path) const;
		std::string MakePathRelativeToBakedFolder(const std::string& Path) const;

	public:
		// a bit more internal asset resolution functions

		template <typename T>
		void ResolveRef(AssetRef<T>& Ref)
		{
			AssetRef<void>& BaseRef = reinterpret_cast<AssetRef<void>&>(Ref);

			ResolveRef(BaseRef, Reflection::FindStruct<T>());
		}

		void ResolveRef(AssetRef<void>& Ref, const Reflection::Struct* Type);

		bool HasPath(const std::string& Path)
		{
			return LoadedAssets.contains(Path);
		}

		template <typename T>
		AssetRef<T> GetRefByPath(const std::string& Path)
		{
			if (!HasPath(Path))
			{
				Log::Error("Trying to GetRefByPath for a non existing path %s", Path.c_str());
				return {};
			}

			AssetData& Data = LoadedAssets[Path];
			if (Data.Type != Reflection::FindStruct<T>())
			{
				Log::Error("Trying to GetRefByPath using the wrong type %s for an asset with path %s and type %s",
					Reflection::FindStruct<T>()->Name,
					Path.c_str(),
					Data.Type->Name);
				return {};
			}

			AssetRef<T> Ref(Path);
			Ref.Resolve();
			return Ref;
		}

		template <typename T>
		void UnloadRef(Columbus::AssetRef<T>& Ref)
		{
			UnloadAssetRaw(Ref.Asset);
			Ref.Asset = nullptr;
		}

		template <typename T>
		T* LoadAssetRaw(const std::string& Path)
		{
			return static_cast<T*>(LoadAssetRaw(Path, Reflection::FindStruct<T>()));
		}

		void* LoadAssetRaw(const std::string& Path, const Reflection::Struct* Type);
		void UnloadAssetRaw(void* Asset);

		void ResolveStructAssetReferences(const Reflection::Struct* Struct, void* Object);

		// register ref from raw data
		template <typename T>
		AssetRef<T> RegisterAssetRef(T* Asset, const std::string& Path)
		{
			if (LoadedAssets.contains(Path))
			{
				// if hit this, it means asset with the same name gets registered twice
				COLUMBUS_ASSERT(false);
				return {};
			}

			// register
			LoadedAssets[Path] = AssetData{ Asset, Reflection::FindStruct<T>(), 1 };
			PathByAsset[Asset] = Path;

			AssetRef<T> Ref;
			Ref.Asset = Asset;
			Ref.Path = Path;

			return Ref;
		}

	public:
		using AssetLoaderFn = std::function<void* (const char* Path)>;
		using AssetUnloaderFn = std::function<void(void* Asset)>;

		std::unordered_map<const Reflection::Struct*, const char*>     AssetExtensions; // for asset picker
		std::unordered_map<const Reflection::Struct*, AssetLoaderFn>   AssetLoaderFunctions;
		std::unordered_map<const Reflection::Struct*, AssetUnloaderFn> AssetUnloaderFunctions;

		std::string SourceDataPath; // path to the source data folder, used for relative paths
		std::string DataPath; // baked data path

	public:
		// DATA
		struct AssetData
		{
			void* Asset = nullptr;
			const Reflection::Struct* Type = nullptr;
			int RefCount = 0;
		};

		std::unordered_map<std::string, AssetData> LoadedAssets;
		std::unordered_map<void*, std::string> PathByAsset; // reverse lookup for asset path by pointer
	};


}
