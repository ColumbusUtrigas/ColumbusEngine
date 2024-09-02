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

	struct GameObject
	{
		Transform Trans;

		GameObjectId Id = -1;
		GameObjectId ParentId = -1;

		std::string Name;

		int MeshId = -1;
		// TODO: components

		std::vector<int> GPUScenePrimitives;

		std::vector<GameObjectId> Children;
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

		// systems
		LightmapSystem Lightmaps;
		PhysicsWorld Physics;

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
