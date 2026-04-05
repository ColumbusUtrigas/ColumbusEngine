#include "Asset.h"
#include "System/Log.h"
#include "Serialisation.h"

#include <filesystem>
#include <fstream>
#include <queue>

#include "Stacktrace.h"

namespace Columbus
{
	static void LinkDependency(AssetRecord* Owner, AssetRecord* Dependency)
	{
		if (Owner == nullptr || Dependency == nullptr || Owner == Dependency)
			return;

		if (std::find(Owner->Dependencies.begin(), Owner->Dependencies.end(), Dependency) == Owner->Dependencies.end())
		{
			Owner->Dependencies.push_back(Dependency);
		}

		if (std::find(Dependency->Dependents.begin(), Dependency->Dependents.end(), Owner) == Dependency->Dependents.end())
		{
			Dependency->Dependents.push_back(Owner);
		}
	}

	static void UnlinkDependencies(AssetRecord* Owner)
	{
		if (Owner == nullptr)
			return;

		for (AssetRecord* Dependency : Owner->Dependencies)
		{
			if (Dependency == nullptr)
				continue;

			auto It = std::remove(Dependency->Dependents.begin(), Dependency->Dependents.end(), Owner);
			Dependency->Dependents.erase(It, Dependency->Dependents.end());
		}

		Owner->Dependencies.clear();
	}

	static void NotifySubscribers(AssetRecord* Record)
	{
		if (Record == nullptr)
			return;

		const std::vector<AssetSubscriber> Subscribers = Record->Subscribers;
		for (const AssetSubscriber& Subscriber : Subscribers)
		{
			if (Subscriber.Callback)
			{
				Subscriber.Callback(Record->Asset);
			}
		}
	}

	static void NotifyDependentsRecursive(AssetRecord* Root)
	{
		if (Root == nullptr)
			return;

		std::unordered_map<AssetRecord*, bool> Visited;
		std::queue<AssetRecord*> Queue;
		Visited[Root] = true;
		Queue.push(Root);

		while (!Queue.empty())
		{
			AssetRecord* Current = Queue.front();
			Queue.pop();

			if (Current != Root)
			{
				NotifySubscribers(Current);
			}

			for (AssetRecord* Dependent : Current->Dependents)
			{
				if (Dependent == nullptr || Visited.contains(Dependent))
					continue;

				Visited[Dependent] = true;
				Queue.push(Dependent);
			}
		}
	}

	static void* LoadPlainStructAsset(const std::string& RealPath, const Reflection::Struct* Type)
	{
		std::ifstream fs(RealPath);
		if (!fs.is_open())
		{
			Log::Error("Couldn't load asset, %s of type %s", RealPath.c_str(), Type->Name);
			return nullptr;
		}

		nlohmann::json json;
		fs >> json;
		fs.close();

		std::string Guid = json["0_type_guid"];
		if (Guid != Type->Guid)
		{
			Log::Error("Asset type mismatch for %s: expected %s, got %s", RealPath.c_str(), Type->Guid, Guid.c_str());
			return nullptr;
		}

		void* Asset = Type->Constructor();
		Reflection_DeserialiseStructJson((char*)Asset, Type, json);
		return Asset;
	}

	static void DestroyAssetObject(AssetSystem& Assets, AssetRecord* Record, void* Asset)
	{
		if (Record == nullptr || Asset == nullptr)
			return;

		auto ReverseIt = Assets.RecordByAsset.find(Asset);
		if (ReverseIt != Assets.RecordByAsset.end() && ReverseIt->second == Record)
		{
			Assets.RecordByAsset.erase(ReverseIt);
		}

		if (Assets.AssetUnloaderFunctions.contains(Record->Type))
		{
			Assets.AssetUnloaderFunctions[Record->Type](Asset);
		}
		else
		{
			Record->Type->Destructor(Asset);
		}
	}

	static void TryDestroyRecord(AssetSystem& Assets, AssetRecord* Record)
	{
		if (Record == nullptr)
			return;

		if (Record->RefCount > 0 || !Record->Subscribers.empty())
			return;

		UnlinkDependencies(Record);
		DestroyAssetObject(Assets, Record, Record->Asset);
		Assets.LoadedAssets.erase(Record->Path);
	}

	static bool is_subpath(const std::filesystem::path& path, const std::filesystem::path& base)
	{
		const auto mismatch_pair = std::mismatch(path.begin(), path.end(), base.begin(), base.end());
		return mismatch_pair.second == base.end();
	}

	bool AssetSystem::IsPathInSourceFolder(const std::string& Path) const
	{
		return is_subpath(Path, SourceDataPath);
	}

	bool AssetSystem::IsPathInBakedFolder(const std::string& Path) const
	{
		return is_subpath(Path, DataPath);
	}

	std::string AssetSystem::MakePathRelativeToSourceFolder(const std::string& Path) const
	{
		std::string r = std::filesystem::relative(Path, SourceDataPath).string();
		std::replace(r.begin(), r.end(), '\\', '/');
		return r;
	}

	std::string AssetSystem::MakePathRelativeToBakedFolder(const std::string& Path) const
	{
		std::string r = std::filesystem::relative(Path, DataPath).string();
		std::replace(r.begin(), r.end(), '\\', '/');
		return r;
	}

	void AssetSystem::ResolveRef(AssetRef<void>& Ref, const Reflection::Struct* Type)
	{
		assert(std::filesystem::path(Ref.Path).is_absolute() == false);

		if (Ref.Path.empty())
		{
			Ref.Record = nullptr;
			return;
		}

		if (LoadedAssets.contains(Ref.Path))
		{
			AssetRecord* Data = LoadedAssets[Ref.Path].get();

			if (Ref.Record == Data)
			{
				return; // already loaded
			}
		}

		AssetRecord* RecordToUnload = Ref.Record;

		LoadAssetRaw(Ref.Path, Type);
		Ref.Record = LoadedAssets.contains(Ref.Path) ? LoadedAssets[Ref.Path].get() : nullptr;
		if (RecordToUnload != nullptr)
		{
			UnloadAssetRaw(RecordToUnload->Asset);
		}
	}

	void* AssetSystem::LoadAssetRaw(const std::string& Path, const Reflection::Struct* Type)
	{
		if (Path.empty())
		{
			Log::Warning("Asset reference is empty in %s", Type->Name);
			return nullptr;
		}

		if (!LoadedAssets.contains(Path))
		{
			std::unique_ptr<AssetRecord> NewRecord = std::make_unique<AssetRecord>();
			NewRecord->Path = Path;
			NewRecord->Type = Type;
			AssetRecord* Record = NewRecord.get();

			LoadedAssets[Path] = std::move(NewRecord);

			auto RealPath = (std::filesystem::path(DataPath) / Path).string();
			std::replace(RealPath.begin(), RealPath.end(), '\\', '/');

			void* Asset = nullptr;
			if (AssetLoaderFunctions.contains(Type))
			{
				// load asset using the registered loader function
				Asset = AssetLoaderFunctions[Type](RealPath.c_str());
			}
			else
			{
				Asset = LoadPlainStructAsset(RealPath, Type);
			}

			if (Asset == nullptr)
			{
				LoadedAssets.erase(Path);
				return nullptr;
			}

			Record->Asset = Asset;
			RecordByAsset[Asset] = Record;
			ResolveStructAssetReferences(Type, Asset, Record);
		}

		AssetRecord* Data = LoadedAssets[Path].get();
		if (Type != Data->Type)
		{
			Log::Error("Asset type mismatch for %s: expected %s, got %s", Path.c_str(), Data->Type->Name, Type->Name);
			return nullptr;
		}

		Data->RefCount++;

		return Data->Asset;
	}

	void AssetSystem::UnloadAssetRaw(void* Asset)
	{
		if (!Asset)
			return;

		if (!RecordByAsset.contains(Asset))
			return;

		AssetRecord* Record = RecordByAsset[Asset];
		Record->RefCount--;
		TryDestroyRecord(*this, Record);
	}

	bool AssetSystem::ReloadAsset(const std::string& Path)
	{
		auto It = LoadedAssets.find(Path);
		if (It == LoadedAssets.end())
			return false;

		AssetRecord* Record = It->second.get();
		if (Record == nullptr || Record->Type == nullptr)
			return false;

		std::string RealPath = (std::filesystem::path(DataPath) / Path).string();
		std::replace(RealPath.begin(), RealPath.end(), '\\', '/');

		void* NewAsset = nullptr;
		if (AssetLoaderFunctions.contains(Record->Type))
		{
			NewAsset = AssetLoaderFunctions[Record->Type](RealPath.c_str());
		}
		else
		{
			NewAsset = LoadPlainStructAsset(RealPath, Record->Type);
		}

		if (NewAsset == nullptr)
			return false;

		void* OldAsset = Record->Asset;

		UnlinkDependencies(Record);
		Record->Asset = NewAsset;
		RecordByAsset[NewAsset] = Record;
		ResolveStructAssetReferences(Record->Type, NewAsset, Record);

		DestroyAssetObject(*this, Record, OldAsset);

		NotifySubscribers(Record);
		NotifyDependentsRecursive(Record);
		return true;
	}

	void AssetSystem::Subscribe(AssetRef<void>& Ref, void* Owner, std::function<void(void* Asset)> Callback)
	{
		if (Owner == nullptr || !Callback)
			return;

		if (Ref.Record == nullptr)
			return;

		Ref.Record->Subscribers.push_back(AssetSubscriber{ Owner, std::move(Callback) });
	}

	void AssetSystem::Unsubscribe(AssetRef<void>& Ref, void* Owner)
	{
		if (Ref.Record == nullptr || Owner == nullptr)
			return;

		auto& Subscribers = Ref.Record->Subscribers;
		auto It = std::remove_if(Subscribers.begin(), Subscribers.end(), [Owner](const AssetSubscriber& Subscriber)
		{
			return Subscriber.Owner == Owner;
		});
		Subscribers.erase(It, Subscribers.end());
		TryDestroyRecord(*this, Ref.Record);
	}

	void AssetSystem::UnsubscribeAllOwnedBy(void* Owner)
	{
		if (Owner == nullptr)
			return;

		std::vector<AssetRecord*> RecordsToDestroy;
		for (auto& [Path, RecordPtr] : LoadedAssets)
		{
			auto& Subscribers = RecordPtr->Subscribers;
			auto It = std::remove_if(Subscribers.begin(), Subscribers.end(), [Owner](const AssetSubscriber& Subscriber)
			{
				return Subscriber.Owner == Owner;
			});

			if (It != Subscribers.end())
			{
				Subscribers.erase(It, Subscribers.end());
				RecordsToDestroy.push_back(RecordPtr.get());
			}
		}

		for (AssetRecord* Record : RecordsToDestroy)
		{
			TryDestroyRecord(*this, Record);
		}
	}

	void AssetSystem::ResolveStructAssetReferences(const Reflection::Struct* Struct, void* Object, AssetRecord* OwnerRecord)
	{
		if (Struct == nullptr || Object == nullptr)
			return;

		for (const auto& Field : Struct->Fields)
		{
			char* FieldData = (char*)Object + Field.Offset;

			if (Field.Type == Reflection::FieldType::AssetRef)
			{
				AssetRef<void>* Ref = (AssetRef<void>*)FieldData;
				ResolveRef(*Ref, Reflection::FindStructByGuid(Field.Typeguid));
				if (OwnerRecord != nullptr && Ref->Record != nullptr)
				{
					LinkDependency(OwnerRecord, Ref->Record);
				}
			}
			else if (Field.Type == Reflection::FieldType::Struct && Field.Struct != nullptr && !Field.Struct->IsNativeBinary)
			{
				ResolveStructAssetReferences(Field.Struct, FieldData, OwnerRecord);
			}
			else if (Field.Type == Reflection::FieldType::Array && Field.Array != nullptr)
			{
				std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
				const int ElementSize = Field.Array->ElementField.Size;
				if (ElementSize <= 0)
					continue;

				const size_t ElementCount = ArrayData->size() / ElementSize;
				for (size_t ElementIndex = 0; ElementIndex < ElementCount; ElementIndex++)
				{
					char* ElementData = ArrayData->data() + ElementIndex * ElementSize;
					const Reflection::Field& ElementField = Field.Array->ElementField;

					if (ElementField.Type == Reflection::FieldType::AssetRef)
					{
						AssetRef<void>* Ref = (AssetRef<void>*)ElementData;
						ResolveRef(*Ref, Reflection::FindStructByGuid(ElementField.Typeguid));
						if (OwnerRecord != nullptr && Ref->Record != nullptr)
						{
							LinkDependency(OwnerRecord, Ref->Record);
						}
					}
					else if (ElementField.Type == Reflection::FieldType::Struct && ElementField.Struct != nullptr && !ElementField.Struct->IsNativeBinary)
					{
						ResolveStructAssetReferences(ElementField.Struct, ElementData, OwnerRecord);
					}
				}
			}
		}
	}

}
