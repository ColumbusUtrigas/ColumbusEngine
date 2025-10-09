#pragma once

#include "GPUScene.h"
#include "Mesh.h"
#include "Lightmaps.h"
#include "Audio/AudioSystem.h"
#include "Graphics/Core/View.h"
#include "Graphics/Particles/ParticleEmitterCPU.h"
#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Scene/Transform.h"
#include "Scene/Project.h"
#include "Physics/PhysicsWorld.h"
#include "Profiling/Profiling.h"
#include "UI/UISystem.h"
#include "Core/Asset.h"

#include "Common/Model/Model.h"
#include <Core/Guid.h>

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTLAS);

DECLARE_CPU_PROFILING_COUNTER(CpuCounter_SceneTransformUpdate);
DECLARE_CPU_PROFILING_COUNTER(CpuCounter_ScenePhysicsUpdate);

namespace Columbus
{

	struct AThing;
	using HStableThingId = TStableSparseArray<AThing*>::Handle;

	struct EngineWorld;

	struct AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AThing);
	public:
		// Common data definition

		Transform Trans;
		Transform TransGlobal;

		EngineWorld* World = nullptr;

		std::string Name;
		HGuid Guid;

		AThing* Parent = nullptr;
		std::vector<AThing*> Children;

		bool bRenderStateDirty = false;
		HStableThingId StableId;

		bool bNeedsTicking = false;
		bool bTransientThing = false; // TODO: remove that - it's a hack to not save transient things in the level

	public:
		virtual ~AThing() {}

		// Common functional definition

		virtual void OnLoad();

		virtual void OnCreate() { Trans.Update(); }
		virtual void OnDestroy() {}

		virtual void OnTick(float DeltaTime) {}

		virtual void OnUpdateRenderState();

		virtual void OnUiPropertyChange();
	};

	template <typename T>
	struct ThingRef
	{
		HGuid Guid;
		T* Thing = nullptr;

		ThingRef()
		{
			Guid = 0;
		}

		operator bool() const { return Thing != nullptr; }
		T* operator->() { return Thing; }
	};


	struct AVolume : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AVolume);
		using Super = AThing;

		Vector4 DebugColour = Vector4(1, 0, 0, 0.3f);
	public:
		bool ContainsPoint(const Vector3& Point) const;
	};

	struct AEffectVolume : public AVolume
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AEffectVolume);
		using Super = AVolume;

	public:
		bool bInfiniteExtent = false;
		float Priority       = 0.0f;
		float BlendWeight    = 1.0f;
		float BlendRadius    = 5.0f;

		HEffectsSettings EffectsSettings;

	public:
		AEffectVolume()
		{
			DebugColour = Vector4(0, 1, 0, 0.3f);
		}

		float ComputeBlendFactor(const Vector3& Point) const;
	};

	struct ALight : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ALight);
		using Super = AThing;

	protected:
		HStableLightId LightHandle;
	public:
		LightType Type = LightType::Point;

		// common
		Vector3 Colour = Vector3(1, 1, 1);
		float Energy = 1.0f;
		float Range = 10.0f;
		float SourceRadius = 0.0f;
		bool  Shadows = false;

		// spot
		float InnerAngle = 40.0f;
		float OuterAngle = 70.0f;

		// area
		Vector2 Size = Vector2(1, 1);
		bool    TwoSided = false;

		// line
		float Length = 1.0f;

	public:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdateRenderState() override;
	};

	struct ADecal : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ADecal);
		using Super = AThing;
	public:
		HStableDecalId DecalHandle;

		AssetRef<Texture2> Texture;
	public:
		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdateRenderState() override;
	};

	struct AMeshInstance : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AMesh);
		using Super = AThing;

	protected:
		std::vector<HStableMeshId> MeshPrimitives;

	public:
		std::vector<Rigidbody*> Rigidbodies;

		AssetRef<Mesh2> Mesh;
		std::vector<AssetRef<Material>> Materials;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdateRenderState() override;
	};

	struct AParticleSystem : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AParticleSystem);
		using Super = AThing;

	public:
		AssetRef<HParticleEmitterSettings> ParticleAsset;
		HParticleEmitterInstanceCPU*       ParticleInstance = nullptr;

		HStableParticlesId ParticleRenderHandle;

	public:
		AParticleSystem();

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnTick(float DeltaTime) override;
		virtual void OnUpdateRenderState() override;
		virtual void OnUiPropertyChange() override;
	};

	struct HLevel
	{
		EngineWorld* World = nullptr;

		std::vector<AThing*> Things;
	};

	struct ALevelThing : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ALevelThing);
		using Super = AThing;

	public:
		AssetRef<HLevel> LevelAsset;
		HLevel* LevelCopy = nullptr;

		std::vector<HStableThingId> ThingsIds;
		std::string PreviousAssetPath;

	public:
		virtual void OnLoad() override;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;

		virtual void OnUiPropertyChange() override;
	};

	struct HWorldIntersectionResult
	{
		bool  bHasIntersection = false;
		float IntersectionFraction = 1.0f; // 0-1 along the distance of the cast

		AThing* HitThing = nullptr;
		Vector3 IntersectionPoint;
		Vector3 IntersectionNormal;
	};


	struct EngineWorld
	{
		// levels, things
		TStableSparseArray<AThing*> AllThings;
		std::unordered_map<u64, HStableThingId> ThingGuidToId; // for fast lookup by guid

		// systems
		AssetSystem& Assets;
		LightmapSystem Lightmaps;
		AudioSystem Audio;
		PhysicsWorld Physics;
		UISystem UI;

		// rendering
		SPtr<DeviceVulkan> Device;
		SPtr<GPUScene> SceneGPU;
		RenderView MainView;

	private:
		std::vector<AEffectVolume*> EffectVolumes;

	public:
		EngineWorld(SPtr<DeviceVulkan> Device);
	public:
		// functions
		ALevelThing* LoadLevelGLTF(const char* Path);
		HLevel* LoadLevelGLTF2(const char* Path);
		HLevel* LoadLevelCLVL(const char* Path);

		void ClearWorld();
		void SaveWorldLevel(const char* Path);

		void AddLevel(HLevel* Level);
		void RemoveLevel(HLevel* Level);

		void ResolveThingThingReferences(AThing* Thing);

		AssetRef<Mesh2> LoadMesh(const Model& MeshModel, const std::string& AssetName);
		AssetRef<Mesh2> LoadMesh(std::span<CPUMeshResource> MeshPrimitives, const std::string& AssetName);
		AssetRef<Mesh2> LoadMesh(const char* AssetPath);

		HStableThingId AddThing(AThing* Thing);
		void DeleteThing(HStableThingId ThingId);

		// give the first found Thing of the type or nullptr if not found
		template <typename T>
		AThing* FindThingByType() { return FindThingByType(Reflection::FindStruct<T>()); }

		// give the first found Thing of the type or nullptr if not found
		AThing* FindThingByType(const Reflection::Struct* Type);

		HWorldIntersectionResult CastRayClosestHit(const Geometry::Ray& Ray, float MaxDistance, int CollisionMask);
		HWorldIntersectionResult CastRayClosestHit(const Vector3& From, const Vector3& To, int CollisionMask);

		// will use MainView camera, screen coordinates must be in 0-1 range
		HWorldIntersectionResult CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates, float MaxDistance, int CollisionMask);

		void BeginFrame();
		void Update(float DeltaTime);
		void UpdateTransforms();
		void PostUpdate();
		void EndFrame();

		float GetGlobalTime() const { return GlobalTime; }

		void FreeResources();

		~EngineWorld()
		{
			FreeResources();
		}

	private:
		float GlobalTime = 0.0f;
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::HLevel, 1, "4112562B-4C50-47FD-B6F4-BAAC28FC4CE7");

CREFLECT_DECLARE_STRUCT_VIRTUAL(Columbus::AThing, 1, "1DE6D316-4F7F-4392-825A-63C77BFF8A85");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AVolume, Columbus::AThing, 1, "EA5F80A9-684B-4F60-95A9-DBE4949B6268");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AEffectVolume, Columbus::AVolume, 1, "1204F071-0B6D-451C-8497-57D78CCD1EF5");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALight, Columbus::AThing, 1, "51A293E0-F98F-47E0-948F-A1D839611B6F");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ADecal, Columbus::AThing, 1, "A809BEA6-6318-4C85-95EE-34414AB36EBB");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AMeshInstance, Columbus::AThing, 1, "ACE7499F-2693-4178-96EB-5D050B7BBD24");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALevelThing, Columbus::AThing, 1, "CBF88292-8650-4764-ACFE-3C84AA6B072C");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AParticleSystem, Columbus::AThing, 1, "FEBD331E-0CCE-47DA-B2F1-4A089EB60DE8");