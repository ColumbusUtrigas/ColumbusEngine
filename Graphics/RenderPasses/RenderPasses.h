#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>

namespace Columbus
{

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
	void RayTracedShadowsPass(RenderGraph& Graph);
	void RenderDeferredLightingPass(RenderGraph& Graph);
	void TonemapPass(RenderGraph& Graph, RenderGraphTextureRef SceneTexture);
	void RenderDeferred(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize); // TODO: View instead of window/camera

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const Camera& MainCamera, const iVector2& WindowSize); // TODO: View instead of window/camera

}
