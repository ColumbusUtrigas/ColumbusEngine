#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	// TODO: View, which contains size, swapchain

	// **********************************
	// Common
	//

	void TonemapPass(RenderGraph& Graph, RenderGraphTextureRef SceneTexture, const iVector2& WindowSize);
	void DebugOverlayPass(RenderGraph& Graph, RenderGraphTextureRef Texture); // TODO: how to specify R/W resources in the render graph?

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

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const iVector2& WindowSize);
	void RenderGBufferPass(RenderGraph& Graph, const Camera& MainCamera, SceneTextures& Textures, const iVector2& WindowSize);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const iVector2& WindowSize, RenderGraphTextureRef ShadowTexture, const SceneTextures& Textures);
	void RenderDeferred(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize); // TODO: View instead of window/camera

	// Real-time raytracing
	//
	RenderGraphTextureRef RayTracedShadowsPass(RenderGraph& Graph, const SceneTextures& Textures, const iVector2& WindowSize);

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize); // TODO: View instead of window/camera

}
