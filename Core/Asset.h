#pragma once

#include "Core/Reflection.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>

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

		template <typename T>
		void ResolveRef(AssetRef<T>& Ref)
		{
			AssetRef<void>& BaseRef = reinterpret_cast<AssetRef<void>&>(Ref);

			ResolveRef(BaseRef, Reflection::FindStruct<T>());
		}

		void ResolveRef(AssetRef<void>& Ref, const Reflection::Struct* Type);

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

	public:
		using AssetLoaderFn = std::function<void* (const char* Path)>;
		using AssetUnloaderFn = std::function<void(void* Asset)>;

		std::unordered_map<const Reflection::Struct*, AssetLoaderFn>   AssetLoaderFunctions;
		std::unordered_map<const Reflection::Struct*, AssetUnloaderFn> AssetUnloaderFunctions;

		std::string SourceDataPath; // path to the source data folder, used for relative paths

	private:
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
