#pragma once

#include "CPUScene.h"
#include "GPUScene.h"
#include "Core/Buffer.h"
#include "Core/Texture.h"
#include "RenderGraph.h"

// vulkan
#include "Vulkan/DeviceVulkan.h"

namespace Columbus
{

	// output of lightmap packer
	struct LightmapMeshVertex
	{
		Vector3 Position;
		Vector3 Normal;
		Vector2 UV2;
	};

	// vertex/index count can be different from the source mesh
	struct LightmapMesh
	{
		// CPU data
		std::vector<LightmapMeshVertex> Vertices;
		std::vector<u32> Indices;

		// GPU data
		Buffer* VertexBuffer = nullptr; // LightmapMeshVertex
		Buffer* IndexBuffer = nullptr;  // u32 index format

		SPtr<DeviceVulkan> Device; // for freeing up resources

		~LightmapMesh();
	};

	struct LightmapAtlas
	{
		u32 Width = 0;
		u32 Height = 0;

		Texture2* Lightmap = nullptr;
	};

	struct LightmapBakingSettings
	{
		int RequestedSamples = 100;
		int Bounces = 3;
		int SamplesPerFrame = 1;
	};

	struct LightmapBakingRenderData
	{
		// textures used for sampling
		RenderGraphTextureRef Normal;
		RenderGraphTextureRef Position;
		RenderGraphTextureRef Validity;

		int AccumulatedSamples = 0;
	};

	struct LightmapSystem
	{
		// assuming there is only one atlas
		// TODO: multiple atlases
		LightmapAtlas Atlas;

		// packed meshes for baking
		std::vector<LightmapMesh> Meshes;

		bool BakingRequested = false;

		// render data
		LightmapBakingSettings BakingSettings;
		LightmapBakingRenderData BakingData;

		void Clear();
	};

	// responsible for generating UV2 and packing atlases
	void GenerateAndPackLightmaps(LightmapSystem& System, CPUScene& Scene);

	// TODO: move that into GenerateAndPackLightmaps?
	void UploadLightmapMeshesToGPU(LightmapSystem& System, SPtr<DeviceVulkan> Device, CPUScene& CpuScene, SPtr<GPUScene> GpuScene);

	// TODO: export/saving

}
