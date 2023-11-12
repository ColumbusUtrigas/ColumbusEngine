#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

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

	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture); // TODO: be able to render debug overlay after render graph execution
	void CopyToSwapchain(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture);

	// **********************************
	// Real-time
	//

	// TODO: create a context for current RG, history and additional resources
	struct HistorySceneTextures
	{
		SPtr<Texture2> Depth;
	};

	struct SceneTextures
	{
		RenderGraphTextureRef GBufferAlbedo;
		RenderGraphTextureRef GBufferNormal;
		RenderGraphTextureRef GBufferWP; // World Position
		RenderGraphTextureRef GBufferRM; // Roughness Metallic
		RenderGraphTextureRef GBufferDS; // Depth Stencil
		RenderGraphTextureRef Velocity;

		HistorySceneTextures& History;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History);
	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures);
	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef ShadowTexture, const SceneTextures& Textures);
	void RenderDeferred(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& HistoryTextures);

	// Real-time raytracing
	//
	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures);

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const RenderView& View);

}
