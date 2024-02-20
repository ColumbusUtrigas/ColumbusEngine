#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "Graphics/IrradianceVolume.h"
#include <Graphics/Lightmaps.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>
#include <vector>

namespace Columbus
{

	// **********************************
	// Common
	//

	void UploadGPUSceneRG(RenderGraph& Graph);

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture);

	void SetupImguiForSwapchain(SPtr<DeviceVulkan> Device, const SwapchainVulkan* Swapchain);
	// TODO: shutdown/clear imgui
	void ShowDebugConsole();
	bool IsDebugConsoleFocused();
	void ShowRenderGraphVisualiser(RenderGraph& Graph);

	void ScreenshotPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef Texture);
	void DebugUIPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture); // TODO: be able to render debug UI after render graph execution
	void CopyToSwapchain(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture);

	// **********************************
	// Upscaling
	//
	// a texture will be created based on DstDesc, but size UpscaleTo will be used
	// that is to support dynamic resolution scaling and re-using one Texture
	RenderGraphTextureRef ApplyFSR1(RenderGraph& Graph, RenderGraphTextureRef Texture, const TextureDesc2& DstDesc, iVector2 UpscaleTo, bool IsHdr, bool UseSharpening, float Sharpening);

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

		RenderGraphTextureRef RTReflections;
		
		HistorySceneTextures& History;
	};

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History);
	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures);
	void PrepareTiledLights(RenderGraph& Graph, const RenderView& View);
	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& HistoryTextures);
	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef OverlayTexture); // only for DebugRender objects
	void RenderDeferred(RenderGraph& Graph, RenderView& View, DeferredRenderContext& HistoryTextures);

	// Real-time raytracing
	//
	void RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext);
	void RayTracedReflectionsPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext);

	// Lightmap baking
	//
	// this pass renders a meshes in lightmap UV space which gives lightmap_texel-vertex correspondance
	// which is then used as sampling points for path tracing
	void PrepareAtlasForLightmapBaking(RenderGraph& Graph, LightmapSystem& System);
	void BakeLightmapPathTraced(RenderGraph& Graph, LightmapSystem& System);

	// Global illumination
	//
	void RenderIndirectLightingDDGI(RenderGraph& Graph, const RenderView& View);

	void RenderIrradianceProbes(RenderGraph& Graph, const RenderView& View, IrradianceVolume& Volume);

	// **********************************
	// Path-Tracing
	//
	void RenderPathTraced(RenderGraph& Graph, const RenderView& View);

}
