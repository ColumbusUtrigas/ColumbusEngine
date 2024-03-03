#pragma once

#include "CPUScene.h"
#include "GPUScene.h"
#include "Lightmaps.h"
#include "Graphics/Core/View.h"
#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Scene/Transform.h"
#include "Profiling/Profiling.h"

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTLAS);

DECLARE_CPU_PROFILING_COUNTER(CpuCounter_SceneTransformUpdate);

namespace Columbus
{

	using GameObjectId = int;

	struct WorldIntersectionResult
	{
		bool HasIntersection;
		int MeshId; // TODO: object ID
		int TriangleId;
		Vector3 IntersectionPoint;
		Geometry::Triangle Triangle;

		static WorldIntersectionResult Invalid()
		{
			return WorldIntersectionResult{ false, -1, -1, { 0, 0, 0 } };
		}
	};

	struct GameObject
	{
		Transform Trans;

		GameObjectId Id = -1;
		GameObjectId ParentId = -1;

		std::string Name; // TODO: optimise

		int MeshId = -1;
		// TODO: material id
		// TODO: components

		// TODO: optimise, use proper data structures, move to World
		std::vector<GameObjectId> Children;
	};

	struct EngineWorld
	{
		// TODO: game objects? proper CPU->GPU routine
		CPUScene SceneCPU;
		SPtr<GPUScene> SceneGPU; // TODO: move some stuff (lights, decals) from SceneGPU here, make a proper upload routine
		std::vector<Box> MeshBoundingBoxes; // TODO: remove/refactor
		std::vector<GameObject> GameObjects; // TODO: currently these are just meshes, so move SceneCPU here

		// systems
		LightmapSystem Lightmaps;

		// rendering
		SPtr<DeviceVulkan> Device;
		RenderView MainView;

		// functions
		void LoadLevelGLTF(const char* Path);

		void ReparentGameObject(GameObjectId Object, GameObjectId NewParent);

		WorldIntersectionResult CastRayClosestHit(const Geometry::Ray& Ray);

		// will use MainView camera, screen coordinates must be in 0-1 range
		WorldIntersectionResult CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates);

		void BeginFrame();
		void UpdateTransforms();
		void EndFrame();

		void FreeResources();

		~EngineWorld()
		{
			FreeResources();
		}
	};

}
