#pragma once

#include "GPUScene.h"
#include "Mesh.h"
#include "Lightmaps.h"
#include "Audio/AudioSystem.h"
#include "Graphics/Core/View.h"
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

	// TODO: delete
	using GameObjectId = int;

	struct EngineWorld;

	// TODO: remove that - temporary thing served it's purpose
	struct GameObject
	{
	public:
		// Common data definition

		Transform Trans;
		EngineWorld* World = nullptr;

		GameObjectId Id = -1;
		GameObjectId ParentId = -1;
		std::vector<GameObjectId> Children;

		std::string Name;

		// TODO: remove that because it's not always a mesh
		int MeshId = -1;
		// TODO: remove that because it's a mesh GPU state thing
		std::vector<int> GPUScenePrimitives;
	};

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

		virtual void OnLoad() {}

		virtual void OnCreate() { Trans.Update(); }
		virtual void OnDestroy() {}

		virtual void OnTick(float DeltaTime) {}

		virtual void OnUpdateRenderState();

		virtual void OnUiPropertyChange();
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

		std::string MeshPath;

		int MeshID = -1; // TODO: proper asset id
		std::vector<int> Materials; // TODO: proper material ids

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
		virtual void OnUpdateRenderState() override;
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

	public:
		virtual void OnLoad() override;

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
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
		SPtr<GPUScene> SceneGPU;


		// TODO: proper resource management for meshes
		std::vector<Mesh2*> Meshes;

		std::unordered_map<std::string, HLevel*> LoadedLevels;

		// TODO: delete
		std::vector<GameObject> GameObjects;

		TStableSparseArray<AThing*> AllThings;

		// systems
		AssetSystem& Assets;
		LightmapSystem Lightmaps;
		AudioSystem Audio;
		PhysicsWorld Physics;
		UISystem UI;

		// rendering
		SkySettings Sky;
		SPtr<DeviceVulkan> Device;
		RenderView MainView;

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

		// TODO: remake it for the asset ref system
		int  LoadMesh(const Model& MeshModel);
		int  LoadMesh(std::span<CPUMeshResource> MeshPrimitives);
		int  LoadMesh(const char* AssetPath);
		void UnloadMesh(int Mesh);
		// TODO: Load/Unload texture
		// TODO: asset referencing system

		HStableThingId AddThing(AThing* Thing);
		void DeleteThing(HStableThingId ThingId);

		// give the first found Thing of the type or nullptr if not found
		template <typename T>
		AThing* FindThingByType() { return FindThingByType(Reflection::FindStruct<T>()); }

		// give the first found Thing of the type or nullptr if not found
		AThing* FindThingByType(const Reflection::Struct* Type);

		// TODO: delete
		GameObjectId CreateGameObject(const char* Name, int Mesh);
		void         DestroyGameObject(GameObjectId Object);
		void         ReparentGameObject(GameObjectId Object, GameObjectId NewParent);

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

CREFLECT_DECLARE_STRUCT(Columbus::Texture2, 1, "1B4AF05B-674A-4B68-8C72-1B46644DA0EC");
CREFLECT_DECLARE_STRUCT(Columbus::Sound, 1, "D8CFCF19-4688-4039-BC40-81C4B796C254");
CREFLECT_DECLARE_STRUCT(Columbus::HLevel, 1, "4112562B-4C50-47FD-B6F4-BAAC28FC4CE7");

CREFLECT_DECLARE_STRUCT_VIRTUAL(Columbus::AThing, 1, "1DE6D316-4F7F-4392-825A-63C77BFF8A85");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALight, Columbus::AThing, 1, "51A293E0-F98F-47E0-948F-A1D839611B6F");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ADecal, Columbus::AThing, 1, "A809BEA6-6318-4C85-95EE-34414AB36EBB");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AMeshInstance, Columbus::AThing, 1, "ACE7499F-2693-4178-96EB-5D050B7BBD24");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALevelThing, Columbus::AThing, 1, "CBF88292-8650-4764-ACFE-3C84AA6B072C");