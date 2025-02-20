#pragma once

#include "GPUScene.h"
#include "Mesh.h"
#include "Lightmaps.h"
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

	using GameObjectId = int;
	using AThingId = int;

	struct WorldIntersectionResult
	{
		bool HasIntersection;
		int ObjectId;
		int MeshPrimitiveId;
		int MeshId;
		int TriangleId;
		Vector3 IntersectionPoint;
		Geometry::Triangle Triangle;

		static WorldIntersectionResult Invalid()
		{
			return WorldIntersectionResult{ false, -1, -1, -1, -1, { 0, 0, 0 } };
		}
	};

	struct EngineWorld;

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
		AThingId Id = -1;

		bool bRenderStateDirty = false;

	public:
		// Common functional definition

		virtual void OnCreate() {}
		virtual void OnDestroy() {}

		// TODO:
		// virtual void OnUpdateRenderState() {}

		virtual void OnUiPropertyChange() {}
	};

	struct ALight : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ALight);
	public:
		using Super = AThing;

		Light L;

	protected:
		// TODO: render state reference

	public:

	};

	struct ADecal : public AThing
	{
		CREFLECT_BODY_STRUCT_VIRTUAL(ADecal);
	public:

		using Super = AThing;

		// reference texture somehow

	public:

	};

	struct EngineWorld
	{
		SPtr<GPUScene> SceneGPU; // TODO: move some stuff (lights, decals) from SceneGPU here, make a proper upload routine

		std::vector<Mesh2*> Meshes; // pointer so that resize of Meshes doesn't invalidate internal pointers
		// TODO: Materials (interface, which will be copied to GPUScene)
		// TODO: Decals
		// TODO: Lights
		// TODO: Textures?
		std::vector<GameObject> GameObjects;

		std::vector<AThing*> AllThings;

		// systems
		LightmapSystem Lightmaps;
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

		// TODO: don't use Model
		int  LoadMesh(const Model& MeshModel);
		int  LoadMesh(std::span<CPUMeshResource> MeshPrimitives);
		void UnloadMesh(int Mesh);

		// TODO: Load/Unload texture
		// TODO: Create/Destroy light source
		// TOOD: Create/Destroy decal
		// TODO: Add/Remove material

		GameObjectId CreateGameObject(const char* Name, int Mesh);
		void         DestroyGameObject(GameObjectId Object);
		void         ReparentGameObject(GameObjectId Object, GameObjectId NewParent);

		WorldIntersectionResult CastRayClosestHit(const Geometry::Ray& Ray);

		// will use MainView camera, screen coordinates must be in 0-1 range
		WorldIntersectionResult CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates);

		void BeginFrame();
		void Update(float DeltaTime);
		void UpdateTransforms();
		void EndFrame();

		void FreeResources();

		~EngineWorld()
		{
			FreeResources();
		}
	};

}

CREFLECT_DECLARE_STRUCT_VIRTUAL(Columbus::AThing, 1, "1DE6D316-4F7F-4392-825A-63C77BFF8A85");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ALight, Columbus::AThing, 1, "51A293E0-F98F-47E0-948F-A1D839611B6F");
CREFLECT_DECLARE_STRUCT_WITH_PARENT_VIRTUAL(Columbus::ADecal, Columbus::AThing, 1, "A809BEA6-6318-4C85-95EE-34414AB36EBB");
