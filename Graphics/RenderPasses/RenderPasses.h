#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	// TODO: View, which contains size, swapchain

	// **********************************
	// Common
	//

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture);

	void InitImguiRendering(SPtr<DeviceVulkan> Device, SwapchainVulkan* Swapchain);
	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture);

	// **********************************
	// Real-time
	//

	struct SceneTextures
	{
		RenderGraphTextureRef GBufferAlbedo;
		RenderGraphTextureRef GBufferNormal;
		RenderGraphTextureRef GBufferWP; // World Position
		RenderGraphTextureRef GBufferRM; // Roughness Metallic
		RenderGraphTextureRef GBufferDS; // Depth Stencil
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View);
	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef ShadowTexture, const SceneTextures& Textures);
	void RenderDeferred(RenderGraph& Graph, const RenderView& View);

	// Real-time raytracing
	//
	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures);

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const RenderView& View);

}
