#include "World.h"

#include <Math/Quaternion.h>
#include <Core/Serialisation.h>
#include <Scene/Project.h>
#include <Scene/TextureAsset.h>
#include <Scene/MeshAsset.h>
#include <Scene/LevelLightingAsset.h>

#include <limits.h>
#include <float.h>
#include <algorithm>
#include <filesystem>
#include <iomanip>
#include <fstream>

IMPLEMENT_MEMORY_PROFILING_COUNTER("Textures", "SceneMemory", MemoryCounter_SceneTextures);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Meshes", "SceneMemory", MemoryCounter_SceneMeshes);
IMPLEMENT_MEMORY_PROFILING_COUNTER("BLAS", "SceneMemory", MemoryCounter_SceneBLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("TLAS", "SceneMemory", MemoryCounter_SceneTLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Irradiance probes", "SceneMemory", MemoryCounter_SceneIrradianceProbes);

IMPLEMENT_CPU_PROFILING_COUNTER("Transforms Update", "SceneCPU", CpuCounter_SceneTransformUpdate);

namespace Columbus
{

	static HIrradianceVolumeBakeBuffer* FindIrradianceVolumeBake(HLevelLightingData& LightingData, u64 OwnerGuid)
	{
		for (HIrradianceVolumeBakeBuffer& Entry : LightingData.IrradianceVolumes)
		{
			if (Entry.Owner.Guid == OwnerGuid)
				return &Entry;
		}

		return nullptr;
	}

	static IrradianceVolume* FindRuntimeIrradianceVolume(GPUScene* Scene, u64 OwnerGuid)
	{
		if (Scene == nullptr)
			return nullptr;

		for (IrradianceVolume& Volume : Scene->IrradianceVolumes)
		{
			if (Volume.OwnerGuid == OwnerGuid)
				return &Volume;
		}

		return nullptr;
	}


	Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data);
	static void DestroyMeshBuffer(SPtr<DeviceVulkan> device, Buffer* buf)
	{
		if (buf)
			RemoveProfilingMemory(MemoryCounter_SceneMeshes, buf->GetSize());
		device->DestroyBufferDeferred(buf);
	}

	static HStableTextureId GetOrAddSceneTexture(GPUScene* Scene, Texture2* Texture)
	{
		if (Scene == nullptr || Texture == nullptr)
			return {};

		HStableTextureId Handle = Scene->Textures.Find(Texture);
		if (Scene->Textures.IsValid(Handle))
			return Handle;

		return Scene->Textures.Add(Texture);
	}

	EngineWorld::EngineWorld(SPtr<DeviceVulkan> InDevice) : Device(InDevice), Assets(AssetSystem::Get())
	{
		Physics.SetGravity(Vector3(0, -9.81f, 0));

		// asset handling callbacks
		{
			Assets.AssetLoaderFunctions[Reflection::FindStruct<Sound>()] = [this](const char* Path) -> void*
			{
				Sound* Snd = new Sound();
				if (!Snd->Load(Path))
				{
					delete Snd;
					return nullptr;
				}
				return Snd;
			};
			Assets.AssetUnloaderFunctions[Reflection::FindStruct<Sound>()] = [this](void* Snd)
			{
				Sound* SoundAsset = static_cast<Sound*>(Snd);
				Audio.MasterMixer.RemoveSourcesUsingSound(SoundAsset);
				delete SoundAsset;
			};
			Assets.AssetExtensions[Reflection::FindStruct<Sound>()] = "wav,ogg,mp3";

			Assets.AssetLoaderFunctions[Reflection::FindStruct<Texture2>()] = [this](const char* Path) -> void*
			{
				Texture2* tex = LoadTextureAssetForRuntime(Device, Path);
				if (!tex)
					return nullptr;
				AddProfilingMemory(MemoryCounter_SceneTextures, tex->GetSize());
				return tex;
			};
			Assets.AssetUnloaderFunctions[Reflection::FindStruct<Texture2>()] = [this](void* Asset)
			{
				Texture2* Texture = static_cast<Texture2*>(Asset);
				while (SceneGPU->Textures.Remove(Texture))
				{
				}
				RemoveProfilingMemory(MemoryCounter_SceneTextures, Texture->GetSize());
				Device->DestroyTextureDeferred(Texture);
			};

			Assets.AssetExtensions[Reflection::FindStruct<Texture2>()] = "cas,dds,png,jpg,jpeg,exr,hdr,tiff,tga,bmp";

			Assets.AssetLoaderFunctions[Reflection::FindStruct<Material>()] = [this](const char* Path) -> void*
			{
				std::ifstream fs(Path);
				if (!fs.is_open())
				{
					Log::Error("Couldn't load material asset %s", Path);
					return nullptr;
				}

				nlohmann::json json;
				fs >> json;
				fs.close();

				const Reflection::Struct* MaterialType = Reflection::FindStruct<Material>();
				std::string Guid = json["0_type_guid"];
				if (Guid != MaterialType->Guid)
				{
					Log::Error("Material asset type mismatch for %s: expected %s, got %s", Path, MaterialType->Guid, Guid.c_str());
					return nullptr;
				}

				Material* Mat = new Material();
				Reflection_DeserialiseStructJson((char*)Mat, MaterialType, json);
				Assets.ResolveStructAssetReferences(MaterialType, Mat);
				RefreshMaterial(Mat);
				return Mat;
			};
			Assets.AssetUnloaderFunctions[Reflection::FindStruct<Material>()] = [this](void* Asset)
			{
				Material* Mat = static_cast<Material*>(Asset);
				SceneGPU->Materials.Remove(Mat->StableId);
				delete Mat;
			};
			Assets.AssetExtensions[Reflection::FindStruct<Material>()] = "mat";

			Assets.AssetUnloaderFunctions[Reflection::FindStruct<Mesh2>()] = [this](void* Asset)
			{
				Mesh2* mesh = static_cast<Mesh2*>(Asset);
				DestroyMeshRuntimeResources(mesh);
				delete mesh;
			};
			Assets.AssetLoaderFunctions[Reflection::FindStruct<Mesh2>()] = [this](const char* Path) -> void*
			{
				Mesh2* Mesh = LoadMeshAssetFromFile(Path);
				if (Mesh == nullptr)
					return nullptr;

				Assets.ResolveStructAssetReferences(Reflection::FindStruct<Mesh2>(), Mesh);
				BuildMeshRuntimeResources(Mesh, std::filesystem::path(Path).filename().string());
				return Mesh;
			};
			Assets.AssetExtensions[Reflection::FindStruct<Mesh2>()] = "cas,obj";

			Assets.AssetLoaderFunctions[Reflection::FindStruct<HLevel>()] = [this](const char* Path) -> void*
			{
				return LoadLevelCLVL(Path);
			};
			Assets.AssetUnloaderFunctions[Reflection::FindStruct<HLevel>()] = [this](void* Asset)
			{
				RemoveLevel((HLevel*)Asset);
				delete (HLevel*)Asset;
			};
			Assets.AssetExtensions[Reflection::FindStruct<HLevel>()] = "clvl";

			Assets.AssetLoaderFunctions[Reflection::FindStruct<HLevelLightingData>()] = [](const char* Path) -> void*
			{
				return LoadLevelLightingDataAssetFromFile(Path);
			};
			Assets.AssetUnloaderFunctions[Reflection::FindStruct<HLevelLightingData>()] = [](void* Asset)
			{
				delete static_cast<HLevelLightingData*>(Asset);
			};
			Assets.AssetExtensions[Reflection::FindStruct<HLevelLightingData>()] = "clight";
		}

		SceneGPU = SPtr<GPUScene>(GPUScene::CreateGPUScene(Device), [this](GPUScene* Scene)
		{
			for (auto& Texture : Scene->Textures)
			{
				Device->DestroyTexture(Texture);
			}

			for (int i = 0; i < Scene->Decals.Size(); i++)
			{
				Device->DestroyTexture(Scene->Decals.Data()[i].Texture);
			}

			GPUScene::DestroyGPUScene(Scene, Device);
		});

		if (Device->SupportsRayTracing())
		{
			AddProfilingMemory(MemoryCounter_SceneTLAS, SceneGPU->TLAS->GetSize());
		}
	}

	static ALevelThing* CreateLevelInstanceRootFromTemplate(EngineWorld* World, AssetRef<HLevel> LevelAsset, bool bFallbackToGeneric)
	{
		LevelAsset.Resolve();

		HLevel* SourceLevel = LevelAsset.Get();
		if (SourceLevel == nullptr)
		{
			Log::Error("Couldn't create level instance for %s", LevelAsset.Path.c_str());
			return nullptr;
		}

		const ALevelThing* Template = SourceLevel->InstanceTemplate;
		ALevelThing* LevelInstance = nullptr;

		if (Template != nullptr)
		{
			const Reflection::Struct* TemplateType = Template->GetTypeVirtual();
			if (TemplateType != nullptr && Reflection::HasParentType(TemplateType, Reflection::FindStruct<ALevelThing>()))
			{
				if (TemplateType->Instantiate)
				{
					LevelInstance = (ALevelThing*)TemplateType->Instantiate((void*)Template);
				}

				if (LevelInstance == nullptr && TemplateType->Constructor)
				{
					Log::Warning("[TypedPrefab] Template type %s cannot instantiate by copy; using default constructor", TemplateType->Name);
					LevelInstance = (ALevelThing*)TemplateType->Constructor();
				}
			}
			else
			{
				Log::Error("[TypedPrefab] Invalid InstanceTemplate type for %s", LevelAsset.Path.c_str());
			}
		}

		if (LevelInstance == nullptr && bFallbackToGeneric)
		{
			LevelInstance = new ALevelThing();
		}
		else if (LevelInstance == nullptr)
		{
			return nullptr;
		}

		LevelInstance->World = World;
		LevelInstance->Parent = nullptr;
		LevelInstance->Children.clear();
		LevelInstance->LevelCopy = nullptr;
		LevelInstance->PreviousAssetPath.clear();
		LevelInstance->StableId = {};
		LevelInstance->Guid = HGuid();
		LevelInstance->bTransientThing = false;
		LevelInstance->bRenderStateDirty = true;
		LevelInstance->bTransformDirty = true;
		LevelInstance->LevelAsset = LevelAsset;

		return LevelInstance;
	}

	static void CopyLevelInstancePlacementState(ALevelThing* Dst, const ALevelThing* Src)
	{
		Dst->Guid = Src->Guid;
		Dst->Name = Src->Name;
		Dst->Trans = Src->Trans;
		Dst->LevelAsset = Src->LevelAsset;
		Dst->Parent = Src->Parent;
		Dst->Children = Src->Children;
		Dst->bTransientThing = Src->bTransientThing;
		Dst->bTransformDirty = true;
		Dst->bRenderStateDirty = true;

		if (Dst->Parent != nullptr)
		{
			for (AThing*& Child : Dst->Parent->Children)
			{
				if (Child == Src)
					Child = Dst;
			}
		}

		for (AThing* Child : Dst->Children)
		{
			if (Child != nullptr && Child->Parent == Src)
				Child->Parent = Dst;
		}
	}

	static AThing* RecreateLevelInstanceFromTemplate(EngineWorld* World, AThing* Thing)
	{
		ALevelThing* LoadedInstance = Reflection::Cast<ALevelThing>(Thing);
		if (LoadedInstance == nullptr)
			return Thing;

		// This can load the referenced prefab level asset.
		LoadedInstance->LevelAsset.Resolve();
		HLevel* SourceLevel = LoadedInstance->LevelAsset.Get();
		if (SourceLevel == nullptr || SourceLevel->InstanceTemplate == nullptr)
			return Thing;

		ALevelThing* TemplateInstance = CreateLevelInstanceRootFromTemplate(World, LoadedInstance->LevelAsset, true);
		assert(TemplateInstance != nullptr);

		CopyLevelInstancePlacementState(TemplateInstance, LoadedInstance);
		delete LoadedInstance;
		return TemplateInstance;
	}

	// Explicit prefab placement path: creates a level-instance root from the asset template,
	// then ALevelThing::OnCreate instantiates the asset's child things.
	ALevelThing* EngineWorld::AddLevelInstance(AssetRef<HLevel> LevelAsset)
	{
		ALevelThing* LevelInstance = CreateLevelInstanceRootFromTemplate(this, LevelAsset, true);
		if (LevelInstance == nullptr)
			return nullptr;

		LevelInstance->OnLoad();
		AddThing(LevelInstance, false, true);

		return LevelInstance;
	}

	static ALevelThing* DeserialiseLevelInstanceTemplate(EngineWorld* World, nlohmann::json& Json)
	{
		if (!Json.contains("InstanceTemplate") || Json["InstanceTemplate"].is_null())
			return nullptr;

		nlohmann::json& TemplateJson = Json["InstanceTemplate"];
		if (!TemplateJson.contains("0_type_guid"))
		{
			Log::Error("Level InstanceTemplate is missing 0_type_guid");
			return nullptr;
		}

		std::string Guid = TemplateJson["0_type_guid"];
		const Reflection::Struct* Type = Reflection::FindStructByGuid(Guid.c_str());
		if (Type == nullptr)
		{
			Log::Error("Level InstanceTemplate type GUID is unknown: %s", Guid.c_str());
			return nullptr;
		}

		if (!Reflection::HasParentType(Type, Reflection::FindStruct<ALevelThing>()))
		{
			Log::Error("Level InstanceTemplate type %s is not an ALevelThing", Type->Name);
			return nullptr;
		}

		ALevelThing* Template = (ALevelThing*)Type->Constructor();
		Reflection_DeserialiseStructJson((char*)Template, Type, TemplateJson);
		Template->World = World;
		World->Assets.ResolveStructAssetReferences(Type, Template);
		return Template;
	}

	static void SerialiseLevelInstanceTemplate(const HLevel& Level, nlohmann::json& Json)
	{
		if (Level.InstanceTemplate == nullptr)
			return;

		nlohmann::json TemplateJson;
		Reflection_SerialiseStructJson<ALevelThing>(*Level.InstanceTemplate, TemplateJson);
		Json["InstanceTemplate"] = TemplateJson;
	}

	HLevel::~HLevel()
	{
		delete InstanceTemplate;
		InstanceTemplate = nullptr;
	}

	HLevel* EngineWorld::LoadLevelCLVL(const char* Path)
	{
		std::filesystem::path LevelPath = Path;
		if (!LevelPath.is_absolute() && !Assets.IsPathInBakedFolder(Path))
		{
			// relative path, prepend project data path
			LevelPath = GCurrentProject->DataPath / LevelPath;
		}

		std::ifstream fs(LevelPath);
		if (!fs.is_open())
		{
			Log::Fatal("Couldn't load scene, %s", LevelPath.c_str());
			return nullptr;
		}

		nlohmann::json json;
		fs >> json;
		fs.close();

		HLevel* Level = new HLevel{};
		Level->World = this;
		Reflection_DeserialiseStructJson(*Level, json);

		Level->InstanceTemplate = DeserialiseLevelInstanceTemplate(this, json);

		if (Level->LightingData.Path.empty())
		{
			const std::string DefaultLightingPath = MakeDefaultLevelLightingDataPath(Assets, LevelPath);
			if (std::filesystem::exists(std::filesystem::path(Assets.DataPath) / DefaultLightingPath))
			{
				Level->LightingData = AssetRef<HLevelLightingData>(DefaultLightingPath);
			}
		}
		Assets.ResolveStructAssetReferences(Reflection::FindStruct<HLevel>(), Level);

		// Actual .clvl load path: deserialize saved things, then rebase level instances from their prefab asset templates.
		for (auto& thing : json["things"])
		{
			AThing* NewThing = Reflection_DeserialiseStructJson_NewInstance<AThing>(thing);
			if (thing["1_Guid"].is_number_unsigned())
			{
				NewThing->Guid = (u64)thing["1_Guid"].get<u64>();
			}
			else
			{
				Log::Warning("Thing %s doesn't have a valid Guid, generating new one", NewThing->Name.c_str());
			}

			NewThing->World = this;
			NewThing = RecreateLevelInstanceFromTemplate(this, NewThing);
			NewThing->World = this;
			NewThing->OnLoad();
			Level->Things.push_back(NewThing);
		}

		return Level;
	}

	void EngineWorld::ClearWorld()
	{
		Audio.Clear();

		while (AllThings.Size() > 0)
		{
			DeleteThing(AllThings[0]->StableId);
		}

	}

	void EngineWorld::SaveWorldLevel(const char* Path, AssetRef<HLevelLightingData> LightingData, const HLevel* SourceMetadata)
	{
		nlohmann::json json;
		HLevel LevelMetadata;
		LevelMetadata.LightingData = LightingData;
		LevelMetadata.InstanceTemplate = SourceMetadata != nullptr ? SourceMetadata->InstanceTemplate : nullptr;
		Reflection_SerialiseStructJson(LevelMetadata, json);
		SerialiseLevelInstanceTemplate(LevelMetadata, json);
		LevelMetadata.InstanceTemplate = nullptr;
		json["things"].array();

		for (int i = 0; i < AllThings.Size(); i++)
		{
			AThing* Thing = AllThings.Data()[i];
			if (Thing->bTransientThing)
			{
				continue;
			}

			auto thing = nlohmann::json();
			if (ALevelThing* LevelThing = Reflection::Cast<ALevelThing>(Thing))
			{
				Reflection_SerialiseStructJson((char*)LevelThing, Reflection::FindStruct<ALevelThing>(), thing);
			}
			else
			{
				Reflection_SerialiseStructJson<AThing>(*Thing, thing);
			}
			thing["1_Guid"] = (u64)Thing->Guid;
			json["things"].push_back(thing);
		}

		std::ofstream fs(Path);
		fs << std::setw(4) << json;

		if (LightingData.IsValid())
		{
			const std::string LightingPath = (std::filesystem::path(Assets.DataPath) / LightingData.Path).string();
			SaveLevelLightingDataAssetToFile(*LightingData.Get(), LightingPath.c_str());
		}

		Log::Message("Saved level to %s", Path);
	}

	void EngineWorld::AddLevel(HLevel* Level)
	{
		for (AThing* Thing : Level->Things)
		{
			Thing->World = this;
			AddThing(Thing, false, false);
		}

		for (AThing* Thing : Level->Things)
		{
			if (!Reflection::HasParentType(Thing->GetTypeVirtual(), Reflection::FindStruct<ALevelThing>()))
			{
				ResolveThingThingReferences(Thing);
			}
		}

		for (AThing* Thing : Level->Things)
		{
			Thing->OnCreate();
			Thing->OnUpdateRenderState();

			if (AEffectVolume* Volume = Reflection::Cast<AEffectVolume>(Thing))
			{
				EffectVolumes.push_back(Volume);
			}
		}

		ApplyLevelLightingData(Level->LightingData);
	}

	void EngineWorld::ApplyLevelLightingData(AssetRef<HLevelLightingData> LightingData)
	{
		if (!LightingData.IsValid() || SceneGPU == nullptr)
			return;

		for (const HIrradianceVolumeBakeBuffer& Entry : LightingData->IrradianceVolumes)
		{
			if (Entry.FormatVersion != IrradianceVolumeBakeFormatVersion || Entry.ProbeStride != sizeof(GPUIrradianceProbe))
				continue;

			IrradianceVolume* Volume = FindRuntimeIrradianceVolume(SceneGPU.get(), (u64)Entry.Owner.Guid);
			if (Volume == nullptr)
				continue;

			const int ExpectedProbeCount = Volume->GetTotalProbes();
			const u64 ExpectedSize = (u64)ExpectedProbeCount * sizeof(GPUIrradianceProbe);
			if (Entry.ProbeCount != ExpectedProbeCount || Entry.ProbeData.Size() != ExpectedSize)
				continue;

			if (Volume->ProbesBuffer != nullptr)
			{
				RemoveProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume->ProbesBufferBytes);
				Volume->ProbesBufferBytes = 0;
				Device->DestroyBufferDeferred(Volume->ProbesBuffer);
				Volume->ProbesBuffer = nullptr;
			}

			BufferDesc ProbeBufferDesc;
			ProbeBufferDesc.BindFlags = BufferType::UAV;
			ProbeBufferDesc.Size = ExpectedSize;
			Volume->ProbesBuffer = Device->CreateBuffer(ProbeBufferDesc, Entry.ProbeData.Data());
			Volume->ProbesBufferBytes = Volume->ProbesBuffer->GetSize();
			AddProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume->ProbesBufferBytes);
		}
	}

	void EngineWorld::QueueIrradianceVolumeBakeReadback(u64 OwnerGuid)
	{
		if (OwnerGuid == 0)
			return;

		if (std::find(PendingIrradianceVolumeBakeReadbacks.begin(), PendingIrradianceVolumeBakeReadbacks.end(), OwnerGuid) == PendingIrradianceVolumeBakeReadbacks.end())
		{
			PendingIrradianceVolumeBakeReadbacks.push_back(OwnerGuid);
		}
	}

	void EngineWorld::FlushPendingIrradianceVolumeBakeReadbacks(AssetRef<HLevelLightingData> LightingData)
	{
		if (PendingIrradianceVolumeBakeReadbacks.empty())
			return;

		if (!LightingData.IsValid())
		{
			PendingIrradianceVolumeBakeReadbacks.clear();
			return;
		}

		u64 TotalBytesWritten = 0;
		int VolumesWritten = 0;
		for (u64 OwnerGuid : PendingIrradianceVolumeBakeReadbacks)
		{
			IrradianceVolume* Volume = FindRuntimeIrradianceVolume(SceneGPU.get(), OwnerGuid);
			if (Volume == nullptr || Volume->ProbesBuffer == nullptr)
				continue;

			const int ProbeCount = Volume->GetTotalProbes();
			const u64 ProbeDataSize = (u64)ProbeCount * sizeof(GPUIrradianceProbe);
			if (ProbeCount <= 0 || ProbeDataSize == 0)
				continue;

			HIrradianceVolumeBakeBuffer* Entry = FindIrradianceVolumeBake(*LightingData.Get(), OwnerGuid);
			if (Entry == nullptr)
			{
				Entry = &LightingData->IrradianceVolumes.emplace_back();
			}

			Entry->Owner.Guid = OwnerGuid;
			Entry->FormatVersion = IrradianceVolumeBakeFormatVersion;
			Entry->ProbeStride = sizeof(GPUIrradianceProbe);
			Entry->ProbeCount = ProbeCount;
			Entry->ProbeData.Bytes.resize((size_t)ProbeDataSize);
			Device->ReadBuffer(Volume->ProbesBuffer, Entry->ProbeData.Data(), ProbeDataSize);
			TotalBytesWritten += ProbeDataSize;
			VolumesWritten++;
		}

		if (!LightingData.Path.empty())
		{
			const std::string LightingPath = (std::filesystem::path(Assets.DataPath) / LightingData.Path).string();
			SaveLevelLightingDataAssetToFile(*LightingData.Get(), LightingPath.c_str());
			Log::Message("Saved irradiance volume bake data: %i volume(s), %llu bytes, %s", VolumesWritten, TotalBytesWritten, LightingPath.c_str());
		}

		PendingIrradianceVolumeBakeReadbacks.clear();
	}

	void EngineWorld::RemoveLevel(HLevel* Level)
	{
		for (AThing* Thing : Level->Things)
		{
			delete Thing;
		}
		Level->Things.clear();
	}

	struct HRuntimeThingRef
	{
		HGuid Guid;
		AThing* Thing = nullptr;
	};

	static const ALevelThing* FindOwningLevelThing(const AThing* Thing)
	{
		for (const AThing* Parent = Thing; Parent != nullptr; Parent = Parent->Parent)
		{
			if (Reflection::HasParentType(Parent->GetTypeVirtual(), Reflection::FindStruct<ALevelThing>()))
				return static_cast<const ALevelThing*>(Parent);
		}

		return nullptr;
	}

	static bool IsAllowedToReferenceTransientThing(const AThing* OwnerThing, const AThing* TargetThing)
	{
		if (TargetThing == nullptr || !TargetThing->bTransientThing)
			return true;

		const ALevelThing* OwnerLevelThing = FindOwningLevelThing(OwnerThing);
		const ALevelThing* TargetLevelThing = FindOwningLevelThing(TargetThing);
		return OwnerLevelThing != nullptr && OwnerLevelThing == TargetLevelThing;
	}

	static void ResolveThingThingReferenceField(EngineWorld* World, AThing* OwnerThing, HRuntimeThingRef* Ref, const Reflection::Field& Field, const std::string& FieldPath)
	{
		AThing* TargetThing = Ref->Thing;
		if (TargetThing == nullptr)
		{
			if (!Ref->Guid.IsValid())
				return;

			auto It = World->ThingGuidToId.find(Ref->Guid);
			if (It == World->ThingGuidToId.end() || !World->AllThings.IsValid(It->second))
			{
				Log::Error("Thing %s field %s has a reference to a non-existing thing with Guid %llu", OwnerThing->Name.c_str(), FieldPath.c_str(), (u64)Ref->Guid);
				return;
			}

			AThing** ppThing = World->AllThings.Get(It->second);
			TargetThing = ppThing ? *ppThing : nullptr;
		}

		if (TargetThing == nullptr)
		{
			Log::Error("Thing %s field %s has a reference to a null thing with Guid %llu", OwnerThing->Name.c_str(), FieldPath.c_str(), (u64)Ref->Guid);
			return;
		}

		const Reflection::Struct* ExpectedType = Reflection::FindStructByGuid(Field.Typeguid);
		if (ExpectedType == nullptr)
		{
			Log::Error("Thing %s field %s has an unknown ThingRef type GUID %s", OwnerThing->Name.c_str(), FieldPath.c_str(), Field.Typeguid ? Field.Typeguid : "<null>");
			return;
		}

		const Reflection::Struct* ActualType = TargetThing->GetTypeVirtual();
		if (!Reflection::HasParentType(ActualType, ExpectedType))
		{
			Log::Error("Thing %s field %s has a reference to %s with wrong type %s, expected %s", OwnerThing->Name.c_str(),
				FieldPath.c_str(), TargetThing->Name.c_str(), ActualType ? ActualType->Name : "<null>", ExpectedType->Name);
			Ref->Thing = nullptr;
			return;
		}

		if (!IsAllowedToReferenceTransientThing(OwnerThing, TargetThing))
		{
			Log::Error("Thing %s field %s cannot reference transient prefab child %s directly", OwnerThing->Name.c_str(),
				FieldPath.c_str(), TargetThing->Name.c_str());
			Ref->Thing = nullptr;
			return;
		}

		Ref->Thing = TargetThing;
	}

	static void ResolveThingThingReferencesInField(EngineWorld* World, AThing* OwnerThing, char* FieldData, const Reflection::Field& Field, const std::string& FieldPath)
	{
		if (Field.Type == Reflection::FieldType::ThingRef)
		{
			ResolveThingThingReferenceField(World, OwnerThing, (HRuntimeThingRef*)FieldData, Field, FieldPath);
			return;
		}

		if (Field.Type == Reflection::FieldType::Struct && Field.Struct != nullptr && !Field.Struct->IsNativeBinary)
		{
			for (const Reflection::Field& StructField : Field.Struct->Fields)
			{
				ResolveThingThingReferencesInField(World, OwnerThing, FieldData + StructField.Offset, StructField, FieldPath + "." + StructField.Name);
			}
			return;
		}

		if (Field.Type == Reflection::FieldType::Array && Field.Array != nullptr)
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			const Reflection::Field& ElementField = Field.Array->ElementField;
			const int ElementSize = ElementField.Size;
			if (ElementSize <= 0)
				return;

			const size_t ElementCount = ArrayData->size() / ElementSize;
			for (size_t ElementIndex = 0; ElementIndex < ElementCount; ElementIndex++)
			{
				char* ElementData = ArrayData->data() + ElementIndex * ElementSize;
				ResolveThingThingReferencesInField(World, OwnerThing, ElementData + ElementField.Offset, ElementField,
					FieldPath + "[" + std::to_string(ElementIndex) + "]");
			}
		}
	}

	void EngineWorld::ResolveThingThingReferences(AThing* Thing)
	{
		for (const Reflection::Field& Field : Thing->GetTypeVirtual()->Fields)
		{
			ResolveThingThingReferencesInField(this, Thing, (char*)Thing + Field.Offset, Field, Field.Name);
		}
	}

	static void RemapPrefabThingReferencesInField(char* FieldData, const Reflection::Field& Field, const std::unordered_map<u64, AThing*>& GuidRemap)
	{
		if (Field.Type == Reflection::FieldType::ThingRef)
		{
			HRuntimeThingRef* Ref = (HRuntimeThingRef*)FieldData;
			auto It = GuidRemap.find((u64)Ref->Guid);
			if (It != GuidRemap.end())
			{
				Ref->Thing = It->second;
			}
			return;
		}

		if (Field.Type == Reflection::FieldType::Struct && Field.Struct != nullptr && !Field.Struct->IsNativeBinary)
		{
			for (const Reflection::Field& StructField : Field.Struct->Fields)
			{
				RemapPrefabThingReferencesInField(FieldData + StructField.Offset, StructField, GuidRemap);
			}
			return;
		}

		if (Field.Type == Reflection::FieldType::Array && Field.Array != nullptr)
		{
			std::vector<char>* ArrayData = (std::vector<char>*)FieldData;
			const Reflection::Field& ElementField = Field.Array->ElementField;
			const int ElementSize = ElementField.Size;
			if (ElementSize <= 0)
				return;

			const size_t ElementCount = ArrayData->size() / ElementSize;
			for (size_t ElementIndex = 0; ElementIndex < ElementCount; ElementIndex++)
			{
				char* ElementData = ArrayData->data() + ElementIndex * ElementSize;
				RemapPrefabThingReferencesInField(ElementData + ElementField.Offset, ElementField, GuidRemap);
			}
		}
	}

	static void RemapPrefabThingReferences(AThing* Thing, const std::unordered_map<u64, AThing*>& GuidRemap)
	{
		for (const Reflection::Field& Field : Thing->GetTypeVirtual()->Fields)
		{
			RemapPrefabThingReferencesInField((char*)Thing + Field.Offset, Field, GuidRemap);
		}
	}

	AssetRef<Mesh2> EngineWorld::LoadMesh(const Model& MeshModel, const std::string& AssetName)
	{
		const SubModel& SModel = MeshModel.GetSubModel(0);

		std::vector<CPUMeshResource> Primitives;
		CPUMeshResource& CPUMesh = Primitives.emplace_back();

		{
			for (int i = 0; i < (int)SModel.VerticesCount; i++)
			{
				CPUMesh.Vertices.push_back(SModel.Positions[i]);
				CPUMesh.Normals.push_back(SModel.Normals[i]);
				CPUMesh.UV1.push_back(SModel.UVs[i]);
				//CPUMesh.Tangents.push_back(Vector4(SModel.Tangents[i], 1));
			}

			for (int i = 0; i < (int)SModel.IndicesCount; i++)
			{
				CPUMesh.Indices.push_back(SModel.Indices[i]);
			}
		}

		return LoadMesh(Primitives, AssetName);
	}

	AssetRef<Mesh2> EngineWorld::LoadMesh(std::span<CPUMeshResource> MeshPrimitives, const std::string& AssetName)
	{
		Mesh2* pMesh = new Mesh2();
		Mesh2& Mesh = *pMesh;
		Mesh.DefaultCollisionSettings.Shape.Type = ECollisionShape::TriMesh;

		for (int PrimitiveIndex = 0; PrimitiveIndex < (int)MeshPrimitives.size(); PrimitiveIndex++)
		{
			MeshPrimitive& Prim = Mesh.Primitives.emplace_back();
			Prim.Name = "Primitive " + std::to_string(PrimitiveIndex);
			Prim.CPU = MeshPrimitives[PrimitiveIndex];
		}

		BuildMeshRuntimeResources(pMesh, AssetName);

		return AssetSystem::Get().RegisterAssetRef<Mesh2>(pMesh, AssetName);
	}

	void EngineWorld::DestroyMeshRuntimeResources(Mesh2* MeshAsset)
	{
		if (MeshAsset == nullptr)
			return;

		for (MeshPrimitive& Primitive : MeshAsset->Primitives)
		{
			DestroyMeshBuffer(Device, Primitive.GPU.Vertices);
			DestroyMeshBuffer(Device, Primitive.GPU.Indices);
			DestroyMeshBuffer(Device, Primitive.GPU.UV1);
			DestroyMeshBuffer(Device, Primitive.GPU.UV2);
			DestroyMeshBuffer(Device, Primitive.GPU.Normals);
			DestroyMeshBuffer(Device, Primitive.GPU.Tangents);
			DestroyMeshBuffer(Device, Primitive.GPU.BoneIndices);
			DestroyMeshBuffer(Device, Primitive.GPU.BoneWeights);
			if (Primitive.GPU.BLAS)
				RemoveProfilingMemory(MemoryCounter_SceneBLAS, Primitive.GPU.BLAS->GetSize());
			Device->DestroyAccelerationStructure(Primitive.GPU.BLAS);
			Primitive.GPU = {};
		}
	}

	void EngineWorld::BuildMeshRuntimeResources(Mesh2* MeshAsset, const std::string& AssetDebugName)
	{
		if (MeshAsset == nullptr)
			return;

		DestroyMeshRuntimeResources(MeshAsset);
		MeshAsset->BoundingBox = Box(Vector3(FLT_MAX), Vector3(-FLT_MAX));
		bool bHasAnyBounds = false;

		for (MeshPrimitive& Primitive : MeshAsset->Primitives)
		{
			CPUMeshResource& CPUMesh = Primitive.CPU;
			GPUMeshResource& GPUMesh = Primitive.GPU;

			if (CPUMesh.Vertices.empty() || CPUMesh.Indices.empty())
				continue;

			if (CPUMesh.UV1.size() != CPUMesh.Vertices.size())
				CPUMesh.UV1.assign(CPUMesh.Vertices.size(), Vector2(0.0f, 0.0f));

			if (CPUMesh.Normals.size() != CPUMesh.Vertices.size())
				CPUMesh.CalculateNormals();

			if (CPUMesh.Tangents.size() != CPUMesh.Vertices.size())
				CPUMesh.CalculateTangents();

			GPUMesh.VertexCount = (u32)CPUMesh.Vertices.size();
			GPUMesh.IndicesCount = (u32)CPUMesh.Indices.size();

			GPUMesh.Vertices = CreateMeshBuffer(Device, sizeof(Vector3) * CPUMesh.Vertices.size(), true, CPUMesh.Vertices.data());
			GPUMesh.Normals = CreateMeshBuffer(Device, sizeof(Vector3) * CPUMesh.Normals.size(), true, CPUMesh.Normals.data());
			GPUMesh.Tangents = CreateMeshBuffer(Device, sizeof(Vector4) * CPUMesh.Tangents.size(), true, CPUMesh.Tangents.data());
			GPUMesh.UV1 = CreateMeshBuffer(Device, sizeof(Vector2) * CPUMesh.UV1.size(), true, CPUMesh.UV1.data());
			GPUMesh.Indices = CreateMeshBuffer(Device, sizeof(u32) * CPUMesh.Indices.size(), true, CPUMesh.Indices.data());

			if (!CPUMesh.UV2.empty())
				GPUMesh.UV2 = CreateMeshBuffer(Device, sizeof(Vector2) * CPUMesh.UV2.size(), true, CPUMesh.UV2.data());
			if (!CPUMesh.BoneIndices.empty())
				GPUMesh.BoneIndices = CreateMeshBuffer(Device, sizeof(iVector4) * CPUMesh.BoneIndices.size(), true, CPUMesh.BoneIndices.data());
			if (!CPUMesh.BoneWeights.empty())
				GPUMesh.BoneWeights = CreateMeshBuffer(Device, sizeof(Vector4) * CPUMesh.BoneWeights.size(), true, CPUMesh.BoneWeights.data());

			Vector3 MinVertex(FLT_MAX);
			Vector3 MaxVertex(-FLT_MAX);
			for (const Vector3& Vertex : CPUMesh.Vertices)
			{
				MinVertex = Vector3::Min(Vertex, MinVertex);
				MaxVertex = Vector3::Max(Vertex, MaxVertex);
			}

			Primitive.BoundingBox = CPUMesh.Vertices.empty() ? Box(Vector3(0.0f), Vector3(0.0f)) : Box(MinVertex, MaxVertex);
			MeshAsset->BoundingBox.Min = Vector3::Min(MeshAsset->BoundingBox.Min, Primitive.BoundingBox.Min);
			MeshAsset->BoundingBox.Max = Vector3::Max(MeshAsset->BoundingBox.Max, Primitive.BoundingBox.Max);
			bHasAnyBounds = true;

			if (MeshAsset->ImportSettings.BuildBLAS)
			{
				AccelerationStructureDesc BlasDesc;
				BlasDesc.Type = AccelerationStructureType::BLAS;
				BlasDesc.Vertices = GPUMesh.Vertices;
				BlasDesc.Indices = GPUMesh.Indices;
				BlasDesc.VerticesCount = GPUMesh.VertexCount;
				BlasDesc.IndicesCount = GPUMesh.IndicesCount;
				GPUMesh.BLAS = Device->CreateAccelerationStructure(BlasDesc);

				if (GPUMesh.BLAS)
				{
					if (!AssetDebugName.empty())
						Device->SetDebugName(GPUMesh.BLAS, (AssetDebugName + " BLAS").c_str());
					AddProfilingMemory(MemoryCounter_SceneBLAS, GPUMesh.BLAS->GetSize());
				}
			}
		}

		if (!bHasAnyBounds)
			MeshAsset->BoundingBox = Box(Vector3(0.0f), Vector3(0.0f));
	}

	void EngineWorld::RefreshMaterial(Material* MaterialAsset)
	{
		if (MaterialAsset == nullptr || SceneGPU == nullptr)
			return;

		MaterialAsset->AlbedoId = GetOrAddSceneTexture(SceneGPU.get(), MaterialAsset->Albedo.Get());
		MaterialAsset->NormalId = GetOrAddSceneTexture(SceneGPU.get(), MaterialAsset->Normal.Get());
		MaterialAsset->OrmId = GetOrAddSceneTexture(SceneGPU.get(), MaterialAsset->Orm.Get());
		MaterialAsset->EmissiveId = GetOrAddSceneTexture(SceneGPU.get(), MaterialAsset->Emissive.Get());

		if (!SceneGPU->Materials.IsValid(MaterialAsset->StableId))
		{
			MaterialAsset->StableId = SceneGPU->Materials.Add(*MaterialAsset);
		}

		Material* SceneMaterial = SceneGPU->Materials.Get(MaterialAsset->StableId);
		if (SceneMaterial)
		{
			*SceneMaterial = *MaterialAsset;
			SceneMaterial->StableId = MaterialAsset->StableId;
		}
	}

	HWorldIntersectionResult EngineWorld::CastRayClosestHit(const Geometry::Ray& Ray, float Distance, int CollisionMask)
	{
		return CastRayClosestHit(Ray.Origin, Ray.Origin + Ray.Direction*Distance, CollisionMask);
	}

	HWorldIntersectionResult EngineWorld::CastRayClosestHit(const Vector3& From, const Vector3& To, int CollisionMask)
	{
		const auto btFrom = btVector3(From.X, From.Y, From.Z);
		const auto btTo = btVector3(To.X, To.Y, To.Z);
		btCollisionWorld::ClosestRayResultCallback Callback(btFrom, btTo);
		Callback.m_collisionFilterMask = CollisionMask;
		Physics.mWorld->rayTest(btFrom, btTo, Callback);

		const auto btN = Callback.m_hitNormalWorld;
		const auto btP = Callback.m_hitPointWorld;

		HWorldIntersectionResult Result;
		Result.bHasIntersection = Callback.hasHit();
		Result.IntersectionFraction = Callback.m_closestHitFraction;
		Result.IntersectionPoint  = Vector3(btP.x(), btP.y(), btP.z());
		Result.IntersectionNormal = Vector3(btN.x(), btN.y(), btN.z());
		Result.HitThing = Callback.m_collisionObject ? (AThing*)Callback.m_collisionObject->getUserPointer() : nullptr;

		return Result;
	}

	HWorldIntersectionResult EngineWorld::CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates, float MaxDistance, int CollisionMask)
	{
		Vector2 NDC = NormalisedScreenCoordinates * 2 - 1;
		NDC.Y = -NDC.Y;

		Geometry::Ray CameraRay{
			.Origin = MainView.CameraCur.Pos,
			.Direction = MainView.CameraCur.CalcRayByNdc(NDC)
		};

		return CastRayClosestHit(CameraRay, MaxDistance, CollisionMask);
	}

	HStableThingId EngineWorld::AddThing(AThing* Thing, bool bResolveReferences, bool bCreate)
	{
		HStableThingId Id = AllThings.Add(Thing);
		ThingGuidToId[Thing->Guid] = Id;

		Thing->StableId = Id;
		Thing->Trans.Update();

		if (Thing->Name.empty())
		{
			Thing->Name = Thing->GetTypeVirtual()->Name + std::string(" ") + std::to_string(AllThings.Size());
		}

		// resolve thing references
		if (bResolveReferences)
		{
			ResolveThingThingReferences(Thing);
		}

		if (bCreate)
		{
			Thing->OnCreate();
			Thing->OnUpdateRenderState();

			if (AEffectVolume* Volume = Reflection::Cast<AEffectVolume>(Thing))
			{
				EffectVolumes.push_back(Volume);
			}
		}

		return Id;
	}

	void EngineWorld::DeleteThing(HStableThingId ThingId)
	{
		if (!AllThings.IsValid(ThingId))
			return;

		AThing* Thing = *AllThings.Get(ThingId);

		Thing->OnDestroy();
		AllThings.Remove(ThingId);
		ThingGuidToId.erase(Thing->Guid);

		if (Thing->Parent)
		{
			Thing->Parent->Children.erase(std::remove(Thing->Parent->Children.begin(), Thing->Parent->Children.end(), Thing));
		}

		// delete all children
		while (!Thing->Children.empty())
		{
			DeleteThing(Thing->Children[0]->StableId);
		}

		if (AEffectVolume* Volume = Reflection::Cast<AEffectVolume>(Thing))
		{
			EffectVolumes.erase(std::remove(EffectVolumes.begin(), EffectVolumes.end(), Volume));
		}

		delete Thing;
	}

	AThing* EngineWorld::FindThingByType(const Reflection::Struct* Type)
	{
		for (AThing* Thing : AllThings)
		{
			if (Thing->GetTypeVirtual() == Type)
				return Thing;
		}

		return nullptr;
	}

	void EngineWorld::BeginFrame()
	{
		ResetProfiling();

		MainView.CameraPrev = MainView.CameraCur;

		for (u32 i = 0; SceneGPU && i < (u32)SceneGPU->Meshes.Size(); i++)
		{
			SceneGPU->Meshes[i].PrevTransform = SceneGPU->Meshes[i].Transform;
		}

		MainView.DebugRender.Clear();
	}

	void EngineWorld::Update(float DeltaTime)
	{
		GlobalTime += DeltaTime;

		for (size_t i = 0; i < AllThings.Size(); i++)
		{
			AThing* Thing = AllThings[i];
			if (Thing->bNeedsTicking)
			{
				Thing->OnTick(DeltaTime);
			}
		}

		Physics.Step(DeltaTime, 48);

		UpdateTransforms();

		// update effects volumes
		HEffectsSettings EffectsSettings = {};
		{
			// sort effect volumes by priority
			std::sort(EffectVolumes.begin(), EffectVolumes.end(), [](const AEffectVolume* A, const AEffectVolume* B) {
				return A->Priority < B->Priority;
			});

			// blend effect volumes
			Vector3 EffectBlendPoint = MainView.CameraCur.Pos;
			for (AEffectVolume* Volume : EffectVolumes)
			{
				float BlendFactor = Volume->ComputeBlendFactor(EffectBlendPoint);

				if (BlendFactor > 0.0f)
				{
					EffectsSettings.BlendWith(Volume->EffectsSettings, BlendFactor);
				}
			}
		}

		SceneGPU->Sky = EffectsSettings.Sky;
		SceneGPU->VolumetricFog = EffectsSettings.VolumetricFog;
		MainView.EffectsSettings = EffectsSettings;
		MainView.UI = &UI;
	}

	void EngineWorld::UpdateTransforms()
	{
		PROFILE_CPU(CpuCounter_SceneTransformUpdate);
	}

	void EngineWorld::PostUpdate()
	{
		for (AThing* Thing : AllThings)
		{
			if (Thing->bNeedsPostPhysicsTicking)
			{
				Thing->OnPostPhysics();
			}
		}

		for (AThing* Thing : AllThings)
		{
			if (Thing->bRenderStateDirty)
			{
				Thing->Trans.Update();
				Thing->OnUpdateRenderState();
				Thing->bRenderStateDirty = false;
			}
		}
	}

	void EngineWorld::EndFrame()
	{
	}

	void EngineWorld::FreeResources()
	{
		SceneGPU = nullptr;
	}

	Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data)
	{
		BufferDesc desc;
		desc.BindFlags = BufferType::UAV;
		desc.Size = size;
		desc.UsedInAccelerationStructure = usedInAS;
		Buffer* result = device->CreateBuffer(desc, data);
		AddProfilingMemory(MemoryCounter_SceneMeshes, result->GetSize());
		return result;
	}
}


// Things implementation
namespace Columbus
{

	bool AThing::IsOwnedByNestedLevel() const
	{
		for (const AThing* ParentThing = Parent; ParentThing != nullptr; ParentThing = ParentThing->Parent)
		{
			if (Reflection::Cast<ALevelThing>(const_cast<AThing*>(ParentThing)))
				return true;
		}

		return false;
	}

	void AThing::OnLoad()
	{
		World->Assets.ResolveStructAssetReferences(GetTypeVirtual(), this);
	}

	void AThing::OnUpdateRenderState()
	{
		Trans.Update();

		// hierarchy
		if (Parent)
		{
			const Transform& ParentWorld = Parent->GetWorldTransform();
			TransGlobal.SetFromMatrix(ParentWorld.GetMatrix() * Trans.GetMatrix());
		}
		else
		{
			TransGlobal = Trans;
		}

		bTransformDirty = false;

		for (auto& Child : Children)
		{
			Child->OnUpdateRenderState();
		}
	}

	void AThing::OnUiPropertyChange()
	{
		bRenderStateDirty = true;
		AssetSystem::Get().ResolveStructAssetReferences(GetTypeVirtual(), this);
	}

	void AThing::SetWorldTransform(const Transform& WorldTransform)
	{
		if (Parent)
		{
			const Transform& ParentWorldTransform = Parent->GetWorldTransform();
			Trans.SetFromMatrix(ParentWorldTransform.GetWorldToLocalMatrix() * WorldTransform.GetMatrix());
		}
		else
		{
			Trans = WorldTransform;
		}
		MarkTransformDirty();
	}

	const Transform& AThing::GetWorldTransform()
	{
		UpdateWorldTransform();
		return TransGlobal;
	}

	void AThing::MarkTransformDirty()
	{
		if (!bTransformDirty)
		{
			bTransformDirty = true;
			// Propagate to children
			for (AThing* child : Children)
			{
				child->MarkTransformDirty();
			}
		}
	}

	void AThing::UpdateWorldTransform()
	{
		if (bTransformDirty)
		{
			Trans.Update();

			if (Parent)
			{
				const Transform& parentWorld = Parent->GetWorldTransform();
				TransGlobal.SetFromMatrix(parentWorld.GetMatrix() * Trans.GetMatrix());
			}
			else
			{
				TransGlobal = Trans;
			}

			bTransformDirty = false;
		}
	}

	bool AVolume::ContainsPoint(const Vector3& Point) const
	{
		const Transform& WorldTransform = const_cast<AVolume*>(this)->GetWorldTransform();
		const Vector3 LocalPoint = (WorldTransform.GetWorldToLocalMatrix() * Vector4(Point, 1.0f)).XYZ();
		return fabsf(LocalPoint.X) <= 0.5f && fabsf(LocalPoint.Y) <= 0.5f && fabsf(LocalPoint.Z) <= 0.5f;
	}

	float AEffectVolume::ComputeBlendFactor(const Vector3& Point) const
	{
		if (bInfiniteExtent)
			return 1.0f;

		Vector3 Center = TransGlobal.Position;
		Vector3 HalfSize = TransGlobal.Scale * 0.5f;
		Box B(Center - HalfSize, Center + HalfSize);

		// hard edge
		if (BlendRadius <= 0.001f)
			return ContainsPoint(Point) ? 1.0f : 0.0f;

		// soft linear falloff

		float Distance = Geometry::PointBoxDistance(Point, B);
		float SafeRadius = Math::Max(BlendRadius, 0.001f);

		if (Distance >= SafeRadius)
			return 0.0f;
		return 1.0f - (Distance / SafeRadius);
	}

	AIrradianceVolume::AIrradianceVolume()
	{
		DebugColour = Vector4(0.15f, 0.65f, 1.0f, 0.25f);
	}

	IrradianceVolume* AIrradianceVolume::FindRuntimeVolume()
	{
		if (World == nullptr || World->SceneGPU == nullptr)
		{
			return nullptr;
		}

		u64 Owner = (u64)Guid;
		for (IrradianceVolume& Volume : World->SceneGPU->IrradianceVolumes)
		{
			if (Volume.OwnerGuid == Owner)
			{
				return &Volume;
			}
		}

		return nullptr;
	}

	IrradianceVolume& AIrradianceVolume::EnsureRuntimeVolume()
	{
		if (IrradianceVolume* Existing = FindRuntimeVolume())
		{
			return *Existing;
		}

		IrradianceVolume Volume{};
		Volume.OwnerGuid = (u64)Guid;
		World->SceneGPU->IrradianceVolumes.push_back(Volume);
		return World->SceneGPU->IrradianceVolumes.back();
	}

	void AIrradianceVolume::RequestBake()
	{
		bBakeRequested = true;
	}

	bool AIrradianceVolume::ConsumeBakeRequest()
	{
		bool Result = bBakeRequested;
		bBakeRequested = false;
		return Result;
	}

	void AIrradianceVolume::OnCreate()
	{
		Super::OnCreate();
		OnUpdateRenderState();
	}

	void AIrradianceVolume::OnDestroy()
	{
		if (World != nullptr && World->SceneGPU != nullptr)
		{
			u64 Owner = (u64)Guid;
			auto& Volumes = World->SceneGPU->IrradianceVolumes;
			for (int i = 0; i < (int)Volumes.size(); i++)
			{
				if (Volumes[i].OwnerGuid == Owner)
				{
					if (Volumes[i].ProbesBuffer != nullptr)
					{
						RemoveProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volumes[i].ProbesBufferBytes);
						Volumes[i].ProbesBufferBytes = 0;
						World->Device->DestroyBufferDeferred(Volumes[i].ProbesBuffer);
					}
					Volumes.erase(Volumes.begin() + i);
					break;
				}
			}
		}

		Super::OnDestroy();
	}

	void AIrradianceVolume::OnUpdateRenderState()
	{
		Super::OnUpdateRenderState();

		ProbeCountX = ProbeCountX < 1 ? 1 : ProbeCountX;
		ProbeCountY = ProbeCountY < 1 ? 1 : ProbeCountY;
		ProbeCountZ = ProbeCountZ < 1 ? 1 : ProbeCountZ;
		RaysPerProbe = RaysPerProbe < 1 ? 1 : RaysPerProbe;
		Bounces = Bounces < 1 ? 1 : Bounces;
		NormalBias = NormalBias < 0.0f ? 0.0f : NormalBias;
		BlendDistance = BlendDistance < 0.0f ? 0.0f : BlendDistance;
		Priority = Priority < 0.0f ? 0.0f : Priority;

		IrradianceVolume& Volume = EnsureRuntimeVolume();
		iVector3 NewProbesCount(ProbeCountX, ProbeCountY, ProbeCountZ);
		const bool bProbeLayoutChanged =
			Volume.ProbesCount.X != NewProbesCount.X ||
			Volume.ProbesCount.Y != NewProbesCount.Y ||
			Volume.ProbesCount.Z != NewProbesCount.Z;

		Volume.Position = TransGlobal.Position;
		Volume.Extent = TransGlobal.Scale;
		Volume.ProbesCount = NewProbesCount;
		Volume.Intensity = Intensity;
		Volume.NormalBias = NormalBias;
		Volume.BlendDistance = BlendDistance;
		Volume.Priority = Priority;
		Volume.bVisualiseProbes = bVisualiseProbes;

		if (bProbeLayoutChanged && Volume.ProbesBuffer != nullptr)
		{
			RemoveProfilingMemory(MemoryCounter_SceneIrradianceProbes, Volume.ProbesBufferBytes);
			Volume.ProbesBufferBytes = 0;
			World->Device->DestroyBufferDeferred(Volume.ProbesBuffer);
			Volume.ProbesBuffer = nullptr;
		}
	}

	void AIrradianceVolume::OnUiPropertyChange()
	{
		Super::OnUiPropertyChange();
		OnUpdateRenderState();
	}

	void ADecal::OnCreate()
	{
		Super::OnCreate();

		Texture.Subscribe(this, [this](Texture2*)
		{
			bRenderStateDirty = true;
		});

		DecalHandle = World->SceneGPU->Decals.Add(GPUDecal());
	}
	void ADecal::OnDestroy()
	{
		World->SceneGPU->Decals.Remove(DecalHandle);

		Super::OnDestroy();
	}
	void ADecal::OnUpdateRenderState()
	{
		Super::OnUpdateRenderState();

		World->SceneGPU->Decals.Get(DecalHandle)->Model = TransGlobal.GetMatrix();
		World->SceneGPU->Decals.Get(DecalHandle)->ModelInverse = TransGlobal.GetMatrix().GetInverted();
		World->SceneGPU->Decals.Get(DecalHandle)->Texture = Texture.Get();
	}

	void ALight::OnCreate()
	{
		Super::OnCreate();
		OnUpdateRenderState();
	}

	void ALight::OnUpdateRenderState()
	{
		Super::OnUpdateRenderState();

		const auto RotMat = TransGlobal.Rotation.ToMatrix();

		GPULight GL{};
		GL.Color = Vector4(Colour * Energy, 1);
		GL.Position = Vector4(TransGlobal.Position, 1);
		GL.Direction = (Vector4(1, 0, 0, 1) * RotMat).Normalized();
		GL.Range = Range;
		GL.SourceRadius = SourceRadius;
		GL.VolumetricIntensity = AffectVolumetricFog ? VolumetricIntensity : 0.0f;
		GL.Type = Type;

		if (Shadows)
			GL.Flags = GL.Flags | ELightFlags::Shadow;
		
		switch (Type)
		{
		case LightType::Spot:
		{
			float inner = cosf(Math::Radians(InnerAngle));
			float outer = cosf(Math::Radians(OuterAngle));

			GL.SizeOrSpotAngles = Vector2(inner, outer);
		}
			break;
		case LightType::Rectangle:
			GL.SizeOrSpotAngles = Size * 0.5f;
			if (TwoSided)
				GL.Flags = GL.Flags | ELightFlags::TwoSided;
			break;
		case LightType::Line:
			GL.SizeOrSpotAngles = Vector2(Length, 0);
			break;
		}

		if (World->SceneGPU->Lights.IsValid(LightHandle))
		{
			World->SceneGPU->UpdateLight(LightHandle, GL);
		}
		else
		{
			LightHandle = World->SceneGPU->AddLight(GL);
		}
	}

	void ALight::OnDestroy()
	{
		World->SceneGPU->RemoveLight(LightHandle);

		Super::OnDestroy();
	}

	void AMeshInstance::CreatePhysicsState()
	{
		if (PhysicsBody)
		{
			World->Physics.RemoveRigidbody(PhysicsBody);
			delete PhysicsBody;

			// TODO: delete shapes
		}

		if (Mesh.Get() == nullptr)
			return;

		const HCollisionSettings& EffectiveCollisionSettings = Mesh->DefaultCollisionSettings;

		btCollisionShape* Shape = Physics::CreatePhysicsShapeFromDesc(EffectiveCollisionSettings.Shape, Mesh.Get());

		bNeedsPostPhysicsTicking = false;

		if (Shape)
		{
			PhysicsBody = new Rigidbody(Shape);

			// when collision or trace is performed, we can find this object
			PhysicsBody->mRigidbody->setUserPointer(this);

			// apply settings from the desc
			PhysicsBody->SetCollisionSettings(EffectiveCollisionSettings);

			World->Physics.AddRigidbody(PhysicsBody);

			if (EffectiveCollisionSettings.MotionType == ECollisionMotionType::Dynamic)
			{
				bNeedsPostPhysicsTicking = true;
				PhysicsBody->Activate();
			}

			PhysicsBody->SetTransform(GetWorldTransform());
		}
	}

	void AMeshInstance::OnPostPhysics()
	{
		Super::OnPostPhysics();

		if (PhysicsBody && PhysicsBody->IsDynamic())
		{
			Transform PhysicsTransform = PhysicsBody->GetTransform();
			SetWorldTransform(PhysicsTransform);
			bRenderStateDirty = true;
		}
	}

	void AMeshInstance::OnCreate()
	{
		Super::OnCreate();
		assert(World != nullptr);
		assert(Mesh.IsValid());

		Mesh.Subscribe(this, [this](Mesh2*)
		{
			if (Mesh.Get() != nullptr)
			{
				for (MeshPrimitive& Prim : Mesh->Primitives)
				{
					if (Prim.DefaultMaterial.IsValid())
					{
						World->RefreshMaterial(Prim.DefaultMaterial.Get());
					}
				}
			}

			for (AssetRef<Material>& MaterialRef : Materials)
			{
				if (MaterialRef.IsValid())
				{
					World->RefreshMaterial(MaterialRef.Get());
				}
			}

			OnDestroy();
			OnCreate();
		});

		for (AssetRef<Material>& MaterialRef : Materials)
		{
			MaterialRef.Subscribe(this, [this](Material* MaterialAsset)
			{
				if (MaterialAsset != nullptr)
				{
					World->RefreshMaterial(MaterialAsset);
				}

				OnDestroy();
				OnCreate();
			});
		}

		int i = 0;
		for (MeshPrimitive& Prim : Mesh->Primitives)
		{
			GPUSceneMesh GPUMesh;
			GPUMesh.MeshResource = &Prim.GPU;
			GPUMesh.Transform = TransGlobal.GetMatrix();
			GPUMesh.PrevTransform = GPUMesh.Transform;

			if (Materials.size() > i && Materials[i].IsValid())
			{
				GPUMesh.MaterialId = Materials[i]->StableId;
			}
			else if (Prim.DefaultMaterial.IsValid())
			{
				GPUMesh.MaterialId = Prim.DefaultMaterial->StableId;
			}

			i++;

			MeshPrimitives.push_back(World->SceneGPU->Meshes.Add(GPUMesh));
		}

		// collision proxy setup
		if (World->WorldType == EWorldType::Game)
		{
			CreatePhysicsState();
		}

		if (PhysicsBody)
		{
			PhysicsBody->mRigidbody->setActivationState(0);
			PhysicsBody->mRigidbody->forceActivationState(0);
		}

		// editor mouse click collision proxy setup
		// TODO: don't do it if no collision - has to be set up, but now default to having trimesh collisions
		if (PhysicsBody == nullptr || World->WorldType == EWorldType::Editor)
		{
			HCollisionShapeDesc EditorCollisionShape;
			EditorCollisionShape.Type = ECollisionShape::TriMesh;

			btCollisionShape* Shape = Physics::CreatePhysicsShapeFromDesc(EditorCollisionShape, Mesh.Get());

			EditorRigidbody = new Rigidbody(Shape);
			EditorRigidbody->SetTransform(GetWorldTransform());
			EditorRigidbody->SetMotionType(ECollisionMotionType::Static);
			EditorRigidbody->mRigidbody->setUserPointer(this);

			World->Physics.AddRigidbody(EditorRigidbody);
		}

	}

	void AMeshInstance::OnUpdateRenderState()
	{
		Super::OnUpdateRenderState();

		for (HStableMeshId Mesh : MeshPrimitives)
		{
			GPUSceneMesh* Proxy = World->SceneGPU->Meshes.Get(Mesh);
			if (Proxy)
			{
				Proxy->Transform = GetWorldTransform().GetMatrix();
			}
		}

		if (EditorRigidbody)
		{
			EditorRigidbody->SetTransform(GetWorldTransform());
		}

		if (PhysicsBody && PhysicsBody->IsKinematic())
		{
			PhysicsBody->SetTransform(GetWorldTransform());
		}
	}

	void AMeshInstance::OnDestroy()
	{
		for (HStableMeshId Mesh : MeshPrimitives)
		{
			World->SceneGPU->Meshes.Remove(Mesh);
		}
		MeshPrimitives.clear();

		if (PhysicsBody)
		{
			World->Physics.RemoveRigidbody(PhysicsBody);
			delete PhysicsBody;
		}
		PhysicsBody = nullptr;

		if (EditorRigidbody)
		{
			World->Physics.RemoveRigidbody(EditorRigidbody);
			delete EditorRigidbody;
		}
		EditorRigidbody = nullptr;

		Super::OnDestroy();
	}

	void ALevelThing::OnLoad()
	{
		Super::OnLoad();

		LevelAsset.Resolve();
	}

	void ALevelThing::OnCreate()
	{
		Super::OnCreate();

		if (LevelAsset.Get() == nullptr)
		{
			Log::Error("Level is null");
			return;
		}

		PreviousAssetPath = LevelAsset.Path;

		assert(LevelCopy == nullptr);

		// instantiate level stuff
		LevelCopy = new HLevel();

		std::unordered_map<AThing*, AThing*> HierarchyRewireMap;
		std::unordered_map<u64, AThing*> GuidRemap;

		if (LevelAsset->InstanceTemplate != nullptr && LevelAsset->InstanceTemplate->Guid.IsValid())
		{
			GuidRemap[(u64)LevelAsset->InstanceTemplate->Guid] = this;
		}

		for (AThing* Thing : LevelAsset->Things)
		{
			// deep copy of a thing
			const Reflection::Struct* Type = Thing->GetTypeVirtual();
			AThing* NewThing = (AThing*)Type->Instantiate(Thing);
			assert(NewThing != nullptr); // things must be able to instantiate

			const HGuid SourceGuid = Thing->Guid;
			LevelCopy->Things.push_back(NewThing);
			NewThing->Guid = HGuid();

			HierarchyRewireMap[Thing] = NewThing;
			if (SourceGuid.IsValid())
			{
				GuidRemap[(u64)SourceGuid] = NewThing;
			}
		}

		// parent-child relationship rewire
		for (AThing* Thing : LevelCopy->Things)
		{
			if (Thing->Parent)
			{
				auto ParentIt = HierarchyRewireMap.find(Thing->Parent);
				Thing->Parent = ParentIt != HierarchyRewireMap.end() ? ParentIt->second : nullptr;
			}

			for (AThing*& Child : Thing->Children)
			{
				auto ChildIt = HierarchyRewireMap.find(Child);
				Child = ChildIt != HierarchyRewireMap.end() ? ChildIt->second : nullptr;
			}

			Thing->Children.erase(std::remove(Thing->Children.begin(), Thing->Children.end(), nullptr), Thing->Children.end());
		}

		RemapPrefabThingReferences(this, GuidRemap);
		for (AThing* Thing : LevelCopy->Things)
		{
			RemapPrefabThingReferences(Thing, GuidRemap);
		}

		// find all root nodes in the level, parent them to the thing
		for (AThing* Thing : LevelCopy->Things)
		{
			Thing->bTransientThing = true;

			if (Thing->Parent == nullptr)
			{
				Thing->Parent = this;
				Children.push_back(Thing);
			}
		}

		World->AddLevel(LevelCopy);

		// AddLevel registers every copied thing before resolving references, so this
		// second pass settles root refs and any child refs that pointed forward.
		World->ResolveThingThingReferences(this);
		for (AThing* Thing : LevelCopy->Things)
		{
			World->ResolveThingThingReferences(Thing);
		}
	}

	void ALevelThing::OnDestroy()
	{
		if (LevelAsset.Get() == nullptr)
		{
			Log::Error("Level is null");
			return;
		}

		assert(LevelCopy != nullptr);

		delete LevelCopy;
		LevelCopy = nullptr;

		Super::OnDestroy();
	}

	void ALevelThing::OnUiPropertyChange()
	{
		Super::OnUiPropertyChange();
		// here dependencies are resolved

		if (LevelAsset.Path == PreviousAssetPath)
			return; // same reference

		// recreate the instance
		OnDestroy();
		OnCreate();
	}


	AParticleSystem::AParticleSystem()
	{
		bNeedsTicking = true;
	}

	void AParticleSystem::OnCreate()
	{
		Super::OnCreate();

		if (ParticleAsset)
		{
			ParticleInstance = new HParticleEmitterInstanceCPU();
			ParticleInstance->Settings = ParticleAsset;

			ParticleRenderHandle = World->SceneGPU->AddParticleSystem(ParticleInstance);
		}
	}

	void AParticleSystem::OnDestroy()
	{
		World->SceneGPU->DeleteParticleSystem(ParticleRenderHandle);

		delete ParticleInstance;
		ParticleInstance = nullptr;

		Super::OnDestroy();
	}

	void AParticleSystem::OnTick(float DeltaTime)
	{
		Super::OnTick(DeltaTime);

		if (!ParticleInstance)
			return;

		ParticleInstance->CurrentPosition = TransGlobal.Position;
		ParticleInstance->CameraPosition = World->MainView.CameraCur.Pos;

		ParticleInstance->Update(DeltaTime);

		bRenderStateDirty = true;
	}

	void AParticleSystem::OnUpdateRenderState()
	{
		Super::OnUpdateRenderState();
	}

	void AParticleSystem::OnUiPropertyChange()
	{
		Super::OnUiPropertyChange();
		// asset dependency resolved

		if (ParticleInstance && ParticleInstance->Settings.Path == ParticleAsset.Path)
			return; // same reference

		// recreate state
		OnDestroy();
		OnCreate();
	}

}

// reflection stuff
using namespace Columbus;

CREFLECT_ENUM_BEGIN(ColourSpaceMode, "")
	CREFLECT_ENUM_FIELD(ColourSpaceMode::Linear, 0)
	CREFLECT_ENUM_FIELD(ColourSpaceMode::SRGB, 1)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(CompressionMode, "")
	CREFLECT_ENUM_FIELD(CompressionMode::None, 0)
	CREFLECT_ENUM_FIELD(CompressionMode::BC1, 1)
	CREFLECT_ENUM_FIELD(CompressionMode::BC3, 2)
	CREFLECT_ENUM_FIELD(CompressionMode::BC5, 3)
	CREFLECT_ENUM_FIELD(CompressionMode::BC6H, 4)
	CREFLECT_ENUM_FIELD(CompressionMode::BC7, 5)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(MipGenMode, "")
	CREFLECT_ENUM_FIELD(MipGenMode::None, 0)
	CREFLECT_ENUM_FIELD(MipGenMode::Default, 1)
	CREFLECT_ENUM_FIELD(MipGenMode::NormalMap, 2)
CREFLECT_ENUM_END()

CREFLECT_ENUM_BEGIN(TextureAddressMode, "")
	CREFLECT_ENUM_FIELD(TextureAddressMode::Repeat, 0)
	CREFLECT_ENUM_FIELD(TextureAddressMode::MirroredRepeat, 1)
	CREFLECT_ENUM_FIELD(TextureAddressMode::ClampToEdge, 2)
CREFLECT_ENUM_END()

CREFLECT_STRUCT_BEGIN(Texture2, "")
	CREFLECT_STRUCT_FIELD(std::string, SourcePath, "Noedit")
	CREFLECT_STRUCT_FIELD(TextureImportSettings, ImportSettings, "")
	CREFLECT_STRUCT_FIELD(TextureStoredPixels, Source, "")
	CREFLECT_STRUCT_FIELD(TextureStoredPixels, Cooked, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(TextureImportSettings, "")
	CREFLECT_STRUCT_FIELD(ColourSpaceMode, ColourSpace, "")
	CREFLECT_STRUCT_FIELD(CompressionMode, Compression, "")
	CREFLECT_STRUCT_FIELD(MipGenMode, MipGen, "")
	CREFLECT_STRUCT_FIELD(TextureAddressMode, AddressMode, "")
	CREFLECT_STRUCT_FIELD(int, MaxSize, "EnterCommits")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(TextureStoredPixels, "")
	CREFLECT_STRUCT_FIELD(int, Width, "Noedit")
	CREFLECT_STRUCT_FIELD(int, Height, "Noedit")
	CREFLECT_STRUCT_FIELD(int, Depth, "Noedit")
	CREFLECT_STRUCT_FIELD(int, Mips, "Noedit")
	CREFLECT_STRUCT_FIELD(std::string, Format, "Noedit")
	CREFLECT_STRUCT_FIELD(std::string, Type, "Noedit")
	CREFLECT_STRUCT_FIELD(Blob, Pixels, "Noedit")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(MeshImportSettings, "")
	CREFLECT_STRUCT_FIELD(bool, GenerateTangents, "")
	CREFLECT_STRUCT_FIELD(float, ImportScale, "")
	CREFLECT_STRUCT_FIELD(bool, BuildBLAS, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(CPUMeshResource, "")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector3, Vertices, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector3, Normals, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector4, Tangents, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector2, UV1, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector2, UV2, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(iVector4, BoneIndices, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(Vector4, BoneWeights, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(u32, Indices, "Noedit")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(MeshPrimitive, "")
	CREFLECT_STRUCT_FIELD(std::string, Name, "")
	CREFLECT_STRUCT_FIELD_ASSETREF(Material, DefaultMaterial, "")
	CREFLECT_STRUCT_FIELD(Box, BoundingBox, "Hidden")
	CREFLECT_STRUCT_FIELD(CPUMeshResource, CPU, "Hidden")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(MeshSocket, "")
	CREFLECT_STRUCT_FIELD(std::string, Name, "")
	CREFLECT_STRUCT_FIELD(Transform, LocalTransform, "")
CREFLECT_STRUCT_END()

static std::string GetMeshPrimitiveArrayLabel(const void* Object, int Index)
{
	const MeshPrimitive* Primitive = reinterpret_cast<const MeshPrimitive*>(Object);
	if (Primitive && !Primitive->Name.empty())
		return Primitive->Name;
	return "Primitive " + std::to_string(Index);
}
CREFLECT_STRUCT_ARRAY_LABEL(MeshPrimitive, GetMeshPrimitiveArrayLabel)

static std::string GetMeshSocketArrayLabel(const void* Object, int Index)
{
	const MeshSocket* Socket = reinterpret_cast<const MeshSocket*>(Object);
	if (Socket && !Socket->Name.empty())
		return Socket->Name;
	return "Socket " + std::to_string(Index);
}
CREFLECT_STRUCT_ARRAY_LABEL(MeshSocket, GetMeshSocketArrayLabel)

CREFLECT_STRUCT_BEGIN(Mesh2, "")
	CREFLECT_STRUCT_FIELD(std::string, SourcePath, "Noedit")
	CREFLECT_STRUCT_FIELD(MeshImportSettings, ImportSettings, "")
	CREFLECT_STRUCT_FIELD(HCollisionSettings, DefaultCollisionSettings, "")
	CREFLECT_STRUCT_FIELD(Box, BoundingBox, "Hidden")
	CREFLECT_STRUCT_FIELD_ARRAY(MeshSocket, Sockets, "")
	CREFLECT_STRUCT_FIELD_ARRAY(MeshPrimitive, Primitives, "Hidden NoArrayResize")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(Material, "")
	CREFLECT_STRUCT_FIELD(Vector4, AlbedoFactor, "Colour")
	CREFLECT_STRUCT_FIELD(Vector4, EmissiveFactor, "Colour HDR")
	CREFLECT_STRUCT_FIELD(MaterialShadingMode, ShadingMode, "")
	CREFLECT_STRUCT_FIELD(float, AlphaCutoff, "SliderMin(0.0) SliderMax(1.0)")
	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Albedo, "")
	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Normal, "")
	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Orm, "")
	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Emissive, "")
	CREFLECT_STRUCT_FIELD(float, Roughness, "SliderMin(0.0) SliderMax(1.0)")
	CREFLECT_STRUCT_FIELD(float, Metallic, "SliderMin(0.0) SliderMax(1.0)")
CREFLECT_STRUCT_END()

CREFLECT_ENUM_BEGIN(MaterialShadingMode, "")
	CREFLECT_ENUM_FIELD(MaterialShadingMode::Opaque, 0)
	CREFLECT_ENUM_FIELD(MaterialShadingMode::Transparent, 1)
	CREFLECT_ENUM_FIELD(MaterialShadingMode::Refractive, 2)
	CREFLECT_ENUM_FIELD(MaterialShadingMode::Masked, 3)
CREFLECT_ENUM_END()

CREFLECT_STRUCT_BEGIN(Sound, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HLevel, "")
	CREFLECT_STRUCT_FIELD_ASSETREF(HLevelLightingData, LightingData, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HIrradianceVolumeBakeBuffer, "")
	CREFLECT_STRUCT_FIELD_THINGREF(AIrradianceVolume, Owner, "")
	CREFLECT_STRUCT_FIELD(int, FormatVersion, "")
	CREFLECT_STRUCT_FIELD(int, ProbeStride, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCount, "")
	CREFLECT_STRUCT_FIELD(Blob, ProbeData, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(HLevelLightingData, "")
	CREFLECT_STRUCT_FIELD_ARRAY(HIrradianceVolumeBakeBuffer, IrradianceVolumes, "")
CREFLECT_STRUCT_END()

CREFLECT_ENUM_BEGIN(LightType, "")
	CREFLECT_ENUM_FIELD(LightType::Directional, 0)
	CREFLECT_ENUM_FIELD(LightType::Point, 1)
	CREFLECT_ENUM_FIELD(LightType::Spot, 2)
	CREFLECT_ENUM_FIELD(LightType::Rectangle, 3)
	CREFLECT_ENUM_FIELD(LightType::Disc, 4)
	CREFLECT_ENUM_FIELD(LightType::Line, 5)
CREFLECT_ENUM_END()


CREFLECT_DEFINE_VIRTUAL(AThing);
CREFLECT_STRUCT_BEGIN(AThing, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(Transform, Trans, "")
	CREFLECT_STRUCT_FIELD(std::string, Name, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AVolume);
CREFLECT_STRUCT_BEGIN(AVolume, "")
CREFLECT_STRUCT_DEFINE_INSTANTIATE()
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AEffectVolume);
CREFLECT_STRUCT_BEGIN(AEffectVolume)
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(bool, bInfiniteExtent, "")
	CREFLECT_STRUCT_FIELD(float, Priority, "")
	CREFLECT_STRUCT_FIELD(float, BlendWeight, "")
	CREFLECT_STRUCT_FIELD(float, BlendRadius, "")
	CREFLECT_STRUCT_FIELD(HEffectsSettings, EffectsSettings, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AIrradianceVolume);
CREFLECT_STRUCT_BEGIN(AIrradianceVolume, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(int, ProbeCountX, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCountY, "")
	CREFLECT_STRUCT_FIELD(int, ProbeCountZ, "")
	CREFLECT_STRUCT_FIELD(int, RaysPerProbe, "")
	CREFLECT_STRUCT_FIELD(int, Bounces, "")
	CREFLECT_STRUCT_FIELD(float, Intensity, "")
	CREFLECT_STRUCT_FIELD(float, NormalBias, "")
	CREFLECT_STRUCT_FIELD(float, BlendDistance, "")
	CREFLECT_STRUCT_FIELD(float, Priority, "")
	CREFLECT_STRUCT_FIELD(bool, bVisualiseProbes, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ALight);
CREFLECT_STRUCT_BEGIN(ALight, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD(LightType, Type, "")

	CREFLECT_STRUCT_FIELD(Vector3,   Colour, "Colour")
	CREFLECT_STRUCT_FIELD(float, Energy, "")
	CREFLECT_STRUCT_FIELD(float, Range, "")
	CREFLECT_STRUCT_FIELD(float, SourceRadius, "")
	CREFLECT_STRUCT_FIELD(bool, Shadows, "")
	CREFLECT_STRUCT_FIELD(bool, AffectVolumetricFog, "")
	CREFLECT_STRUCT_FIELD(float, VolumetricIntensity, "")

	CREFLECT_STRUCT_FIELD(float, InnerAngle, "")
	CREFLECT_STRUCT_FIELD(float, OuterAngle, "")

	CREFLECT_STRUCT_FIELD(Vector2, Size, "")
	CREFLECT_STRUCT_FIELD(bool, TwoSided, "")

	CREFLECT_STRUCT_FIELD(float, Length, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ADecal);
CREFLECT_STRUCT_BEGIN(ADecal, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD_ASSETREF(Texture2, Texture, "");
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AMeshInstance);
CREFLECT_STRUCT_BEGIN(AMeshInstance, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD_ASSETREF(Mesh2, Mesh, "");
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ALevelThing);
CREFLECT_STRUCT_BEGIN(ALevelThing, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD_ASSETREF(HLevel, LevelAsset, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AParticleSystem)
CREFLECT_STRUCT_BEGIN(AParticleSystem, "")
	CREFLECT_STRUCT_DEFINE_INSTANTIATE()
	CREFLECT_STRUCT_FIELD_ASSETREF(HParticleEmitterSettings, ParticleAsset, "")
CREFLECT_STRUCT_END()
