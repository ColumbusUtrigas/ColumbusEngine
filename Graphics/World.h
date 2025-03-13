#pragma once

#include "GPUScene.h"
#include "Mesh.h"
#include "Lightmaps.h"
#include "Audio/AudioSystem.h"
#include "Graphics/Core/View.h"
#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Scene/Transform.h"
#include "Physics/PhysicsWorld.h"
#include "Profiling/Profiling.h"
#include "UI/UISystem.h"

#include "Common/Model/Model.h"

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTLAS);

DECLARE_CPU_PROFILING_COUNTER(CpuCounter_SceneTransformUpdate);
DECLARE_CPU_PROFILING_COUNTER(CpuCounter_ScenePhysicsUpdate);

namespace Columbus
{

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
		EngineWorld* World = nullptr;

		std::string Name;

		bool bRenderStateDirty = false;

	public:
		// Common functional definition

		virtual void OnCreate() { Trans.Update(); }
		virtual void OnDestroy() {}

		virtual void OnUpdateRenderState() {}

		virtual void OnUiPropertyChange() { bRenderStateDirty = true; }
	};

	struct ALight : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ALight);
		using Super = AThing;

	protected:
		HStableLightId LightHandle;
	public:
		Light L;

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
	public:

	};

	struct AMeshInstance : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(AMesh);
		using Super = AThing;
	public:
		int MeshID = -1; // TODO: proper asset id
		std::vector<int> Materials; // TODO: proper material ids

		virtual void OnCreate() override;
		virtual void OnDestroy() override;
	};

	using HStableThingId = TStableSparseArray<AThing*>::Handle;

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

		std::vector<Mesh2*> Meshes; // pointer so that resize of Meshes doesn't invalidate internal pointers
		std::vector<Sound*> Sounds;
		// TODO: texture resources

		std::vector<GameObject> GameObjects;

		TStableSparseArray<AThing*> AllThings;

		// systems
		LightmapSystem Lightmaps;
		AudioSystem Audio;
		PhysicsWorld Physics;
		UISystem UI;

		// rendering
		SkySettings Sky;
		SPtr<DeviceVulkan> Device;
		RenderView MainView;

	public:
		EngineWorld();
	public:
		// functions
		void LoadLevelGLTF(const char* Path);

		// TODO: remake it for the asset ref system
		int  LoadMesh(const Model& MeshModel);
		int  LoadMesh(std::span<CPUMeshResource> MeshPrimitives);
		int  LoadMesh(const char* AssetPath);
		void UnloadMesh(int Mesh);
		// TODO: Load/Unload texture
		// TODO: asset referencing system

		Sound* LoadSoundAsset(const char* AssetPath);
		void   UnloadSoundAsset(Sound* Snd);

		HStableThingId AddThing(AThing* Thing);
		void DeleteThing(HStableThingId ThingId);

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

CREFLECT_DECLARE_STRUCT_VIRTUAL(Columbus::AThing, 1, "1DE6D316-4F7F-4392-825A-63C77BFF8A85");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALight, Columbus::AThing, 1, "51A293E0-F98F-47E0-948F-A1D839611B6F");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ADecal, Columbus::AThing, 1, "A809BEA6-6318-4C85-95EE-34414AB36EBB");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::AMeshInstance, Columbus::AThing, 1, "ACE7499F-2693-4178-96EB-5D050B7BBD24");