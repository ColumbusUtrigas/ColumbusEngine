#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

	// **********************************
	// Common
	//

	void TonemapPass(RenderGraph& Graph, RenderGraphTextureRef SceneTexture);

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
	void RenderGBufferPass(RenderGraph& Graph, const Camera& MainCamera, SceneTextures& Textures);
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
