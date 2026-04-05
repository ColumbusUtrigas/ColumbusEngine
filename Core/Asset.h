#pragma once

#include "Core/Reflection.h"
#include "Core/Assert.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

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

	struct AssetSubscriber
	{
		void* Owner = nullptr;
		std::function<void(void* Asset)> Callback;
	};

	struct AssetRecord
	{
		std::string Path;
		const Reflection::Struct* Type = nullptr;
		void* Asset = nullptr;
		int RefCount = 0;

		std::vector<AssetRecord*> Dependencies;
		std::vector<AssetRecord*> Dependents;
		std::vector<AssetSubscriber> Subscribers;
	};

	template <typename T>
	struct AssetRef
	{
		std::string Path;
		AssetRecord* Record = nullptr;

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

		AssetRef(AssetRef<T>&& Other) noexcept
		{
			Path = std::move(Other.Path);
			Record = Other.Record;

			Other.Record = nullptr;
			Other.Path.clear();
		}

		AssetRef<T>& operator=(const AssetRef<T>& Other)
		{
			if (this == &Other)
			{
				return *this;
			}

			Unload();
			Path = Other.Path;

			if (Other.Record != nullptr)
			{
				Resolve(); // increase refcount
			}

			return *this;
		}

		AssetRef<T>& operator=(AssetRef<T>&& Other) noexcept
		{
			if (this == &Other)
			{
				return *this;
			}

			Unload();

			Path = std::move(Other.Path);
			Record = Other.Record;

			Other.Record = nullptr;
			Other.Path.clear();

			return *this;
		}

		T* Get()
		{
			return Record != nullptr ? static_cast<T*>(Record->Asset) : nullptr;
		}

		const T* Get() const
		{
			return Record != nullptr ? static_cast<const T*>(Record->Asset) : nullptr;
		}

		T* operator->()
		{
			return Get();
		}

		const T* operator->() const
		{
			return Get();
		}

		operator bool() const
		{
			return IsValid();
		}

		bool IsValid() const
		{
			return Get() != nullptr;
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

		void Subscribe(void* Owner, std::function<void(T* Asset)> Callback)
		{
			AssetSystem::Get().Subscribe(*this, Owner, [Callback = std::move(Callback)](void* Asset)
			{
				Callback(static_cast<T*>(Asset));
			});
		}

		void Unsubscribe(void* Owner)
		{
			AssetSystem::Get().Unsubscribe(*this, Owner);
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

			AssetRecord* Data = LoadedAssets[Path].get();
			if (Data->Type != Reflection::FindStruct<T>())
			{
				Log::Error("Trying to GetRefByPath using the wrong type %s for an asset with path %s and type %s",
					Reflection::FindStruct<T>()->Name,
					Path.c_str(),
					Data->Type->Name);
				return {};
			}

			AssetRef<T> Ref(Path);
			Ref.Resolve();
			return Ref;
		}

		template <typename T>
		void UnloadRef(Columbus::AssetRef<T>& Ref)
		{
			UnloadRef(reinterpret_cast<AssetRef<void>&>(Ref));
		}

		void UnloadRef(AssetRef<void>& Ref)
		{
			if (Ref.Record != nullptr)
			{
				UnloadAssetRaw(Ref.Record->Asset);
			}

			Ref.Record = nullptr;
		}

		template <typename T>
		T* LoadAssetRaw(const std::string& Path)
		{
			return static_cast<T*>(LoadAssetRaw(Path, Reflection::FindStruct<T>()));
		}

		void* LoadAssetRaw(const std::string& Path, const Reflection::Struct* Type);
		void UnloadAssetRaw(void* Asset);

		bool ReloadAsset(const std::string& Path);

		void ResolveStructAssetReferences(const Reflection::Struct* Struct, void* Object, AssetRecord* OwnerRecord = nullptr);

		template <typename T>
		void Subscribe(AssetRef<T>& Ref, void* Owner, std::function<void(void* Asset)> Callback)
		{
			if (Ref.Record == nullptr && !Ref.Path.empty())
			{
				ResolveRef(Ref);
			}

			Subscribe(reinterpret_cast<AssetRef<void>&>(Ref), Owner, std::move(Callback));
		}

		void Subscribe(AssetRef<void>& Ref, void* Owner, std::function<void(void* Asset)> Callback);

		template <typename T>
		void Unsubscribe(AssetRef<T>& Ref, void* Owner)
		{
			Unsubscribe(reinterpret_cast<AssetRef<void>&>(Ref), Owner);
		}

		void Unsubscribe(AssetRef<void>& Ref, void* Owner);
		void UnsubscribeAllOwnedBy(void* Owner);

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
			std::unique_ptr<AssetRecord> Record = std::make_unique<AssetRecord>();
			Record->Path = Path;
			Record->Type = Reflection::FindStruct<T>();
			Record->Asset = Asset;
			Record->RefCount = 1;
			AssetRecord* RecordPtr = Record.get();

			LoadedAssets[Path] = std::move(Record);
			RecordByAsset[Asset] = RecordPtr;
			ResolveStructAssetReferences(RecordPtr->Type, Asset, RecordPtr);

			AssetRef<T> Ref;
			Ref.Record = RecordPtr;
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
		std::unordered_map<std::string, std::unique_ptr<AssetRecord>> LoadedAssets;
		std::unordered_map<void*, AssetRecord*> RecordByAsset; // reverse lookup for asset record by pointer
	};


}
