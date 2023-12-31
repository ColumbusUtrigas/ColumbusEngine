#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>
#include <vector>

namespace Columbus
{

	// TODO: internal render resolution, upscaling

	// **********************************
	// Common
	//

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture);

	void SetupImguiForSwapchain(SPtr<DeviceVulkan> Device, const SwapchainVulkan* Swapchain);
	// TODO: shutdown/clear imgui
	void ShowDebugConsole();
	bool IsDebugConsoleFocused();
	void ShowRenderGraphVisualiser(RenderGraph& Graph);

	void DebugUIPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture); // TODO: be able to render debug UI after render graph execution
	void CopyToSwapchain(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture);

	// **********************************
	// Real-time
	//

	struct GPULightRenderInfo
	{
		RenderGraphTextureRef RTShadow;
		// TODO: denoising history textures should go here as well, think about persisting and clearing them
	};

	struct HistorySceneTextures
	{
		SPtr<Texture2> Depth;
	};

	struct DeferredRenderContext
	{
		std::vector<GPULightRenderInfo> LightRenderInfos;

		HistorySceneTextures History;
	};

	struct SceneTextures
	{
		RenderGraphTextureRef GBufferAlbedo;
		RenderGraphTextureRef GBufferNormal;
		RenderGraphTextureRef GBufferWP; // World Position
		RenderGraphTextureRef GBufferRM; // Roughness Metallic
		RenderGraphTextureRef GBufferDS; // Depth Stencil
		RenderGraphTextureRef Velocity;
		RenderGraphTextureRef Lightmap;
		
		HistorySceneTextures& History;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History);
	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures);
	void PrepareTiledLights(RenderGraph& Graph, const RenderView& View);
	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& HistoryTextures);
	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef OverlayTexture); // only for DebugRender objects
	void RenderDeferred(RenderGraph& Graph, const RenderView& View, DeferredRenderContext& HistoryTextures);

	// Real-time raytracing
	//
	void RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext);

	// Lightmap baking
	//
	struct LightmapBakingSettings
	{
		int LightmapSize;
		int RequestedSamples;
		int Bounces = 3;
		int SamplesPerFrame = 1;
		u32 MeshIndex;
	};

	struct LightmapBakingRenderData
	{
		// textures used for sampling
		RenderGraphTextureRef Albedo;
		RenderGraphTextureRef Normal;
		RenderGraphTextureRef WP;
		RenderGraphTextureRef RM;
		RenderGraphTextureRef Validity;

		Texture2* Lightmap;

		LightmapBakingSettings Settings;
		int AccumulatedSamples = 0;
	};

	LightmapBakingRenderData CreateLightmapBakingData(SPtr<DeviceVulkan> Device, LightmapBakingSettings Settings);

	// this pass renders a mesh in lightmap UV space which gives lightmap_texel-vertex correspondance
	// which is then used as sampling points for path tracing
	void                     PrepareMeshForLightmapBaking(RenderGraph& Graph, const RenderView& View, LightmapBakingRenderData& Data);
	void                     BakeLightmapPathTraced(RenderGraph& Graph, LightmapBakingRenderData& Data);

	// Global illumination
	//
	void RenderIndirectLightingDDGI(RenderGraph& Graph, const RenderView& View);

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const RenderView& View);

}
