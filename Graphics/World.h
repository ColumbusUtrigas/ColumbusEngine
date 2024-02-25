#pragma once

#include "CPUScene.h"
#include "GPUScene.h"
#include "Lightmaps.h"
#include "Graphics/Core/View.h"
#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Profiling/Profiling.h"

DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTLAS);

namespace Columbus
{

	struct WorldIntersectionResult
	{
		bool HasIntersection;
		int MeshId;
		int TriangleId;
		Vector3 IntersectionPoint;
		Geometry::Triangle Triangle;

		static WorldIntersectionResult Invalid()
		{
			return WorldIntersectionResult{ false, -1, -1, { 0, 0, 0 } };
		}
	};

	struct EngineWorld
	{
		// TODO: game objects? proper CPU->GPU routine
		CPUScene SceneCPU;
		SPtr<GPUScene> SceneGPU;
		std::vector<Box> MeshBoundingBoxes; // TODO: remove/refactor

		// systems
		LightmapSystem Lightmaps;

		// rendering
		SPtr<DeviceVulkan> Device;
		RenderView MainView;

		// functions
		void LoadLevelGLTF(const char* Path);

		WorldIntersectionResult CastRayClosestHit(const Geometry::Ray& Ray);

		// will use MainView camera, screen coordinates must be in 0-1 range
		WorldIntersectionResult CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates);

		void BeginFrame();
		void EndFrame();

		void FreeResources();

		~EngineWorld()
		{
			FreeResources();
		}
	};

}
