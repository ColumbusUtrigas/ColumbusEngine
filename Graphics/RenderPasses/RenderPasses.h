#pragma once

#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/View.h"
#include "BakedLightingRuntime.h"
#include "Graphics/ShadowMaps.h"
#include "RadianceCache.h"
#include <Graphics/Lightmaps.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/Camera.h>
#include <vector>

namespace Columbus
{

	// **********************************
	// Common
	//

	struct DeferredRenderContext;
	void UploadGPUSceneRG(RenderGraph& Graph, const RenderView& View, DeferredRenderContext* DeferredContext = nullptr);
	void PrepareDeferredView(RenderView& View);

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture);

	// TODO: move to a more appropriate place
	void ShowDebugStats(bool* Open = nullptr);
	void ShowLoadedAssets(bool* Open = nullptr);
	void ShowDebugConsole(bool bShowAuxiliaryWindows = true);
	bool IsDebugConsoleFocused();
	void ShowRenderGraphVisualiser(RenderGraph& Graph, bool* Open = nullptr);

	void ScreenshotPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef Texture);

	void ShaderMemsetTexture(RenderGraph& Graph, RenderGraphTextureRef Texture, Vector4 Value);

	// **********************************
	// Upscaling
	//
	// a texture will be created based on DstDesc, but size UpscaleTo will be used
	// that is to support dynamic resolution scaling and re-using one Texture
	RenderGraphTextureRef ApplyFSR1(RenderGraph& Graph, RenderGraphTextureRef Texture, const TextureDesc2& DstDesc, iVector2 UpscaleTo, bool IsHdr, bool UseSharpening, float Sharpening);

	// **********************************
	// Real-time
	//

	struct RTGIDenoiserHistory
	{
		Texture2* Radiance = nullptr;
		Texture2* AverageRadiance = nullptr;
		Texture2* Variance = nullptr;
		Texture2* SampleCount = nullptr;

		void Destroy(SPtr<DeviceVulkan> Device);
	};

	struct GPULightRenderInfo
	{
		RenderGraphTextureRef RTShadow;
		// RT histories are owned by DeferredRenderContext::Shadows and keyed by stable light identity.
	};

	struct HistorySceneTextures
	{
		Texture2* Depth = nullptr;
		Texture2* RoughnessMetallic = nullptr;
		Texture2* Normals = nullptr;
		Texture2* VolumetricFogFroxels = nullptr;

		Texture2* TAAHistory = nullptr;
		Texture2* RTReflectionsRadiance = nullptr;
		Texture2* RTReflectionsAverageRadiance = nullptr;
		Texture2* RTReflectionsVariance = nullptr;
		Texture2* RTReflectionsSampleCount = nullptr;

		RTGIDenoiserHistory RTGI_History;

		void Destroy(SPtr<DeviceVulkan> Device);
	};

	// just some type erasure to reduce nested includes, this struct shouldn't be accessed from outside
	struct FidelityFXContext
	{
		void* InterfaceFFX = nullptr; // FfxInterface*
		void* BlurContext  = nullptr; // FfxBlurContext*
		void* DofState     = nullptr; // InternalFfxDofState*
	};

	struct DeferredRenderContext;

	void InitDeferredRenderContext(SPtr<DeviceVulkan> Device, DeferredRenderContext* pContext);
	void ShutdownDeferredRenderContext(SPtr<DeviceVulkan> Device, DeferredRenderContext* pContext);

	struct DeferredRenderContext
	{
		SPtr<DeviceVulkan> Device;

		std::vector<GPULightRenderInfo> LightRenderInfos; // Compact RT mask list, not indexed by scene light.
		ShadowRenderState               Shadows;
		RadianceCache::State            RadianceCache;

		// Rebuilt for the current view/frame; the render graph owns the pooled buffer.
		RenderGraphBufferRef LightTiles = -1;

		// prepared once per view per frame, read by render passes
		BakedLightingPrepared BakedLighting;

		EDeferredRenderVisualisationMode VisualisationMode = EDeferredRenderVisualisationMode::Final;
		HistorySceneTextures History;

		FidelityFXContext* FFX = nullptr;

		DeferredRenderContext(SPtr<DeviceVulkan> Device) : Device(Device)
		{
			InitDeferredRenderContext(Device, this);
		}

		~DeferredRenderContext()
		{
			ShutdownDeferredRenderContext(Device, this);
		}
	};

	struct SceneTextures
	{
		RenderGraphTextureRef GBufferAlbedo;
		RenderGraphTextureRef GBufferNormal;
		RenderGraphTextureRef GBufferRM; // Roughness Metallic
		RenderGraphTextureRef GBufferEmissive;
		RenderGraphTextureRef GBufferDS; // Depth Stencil
		RenderGraphTextureRef Velocity;
		RenderGraphTextureRef Lightmap;

		RenderGraphTextureRef RTReflections;
		RenderGraphTextureRef RTGI;
		RenderGraphTextureRef VolumetricFog;

		RenderGraphTextureRef FinalBeforeTonemap;
		RenderGraphTextureRef FinalAfterTonemap;

		HistorySceneTextures& History;

		RadianceCache::RadianceCacheData RadianceCache;
	};

	// FidelityFX wrapper
	namespace FFX
	{
		// values match FfxBlurKernelPermutation
		enum class EFFXBlurSigma
		{
			k1_6 = 1, // FFX_BLUR_KERNEL_PERMUTATION_0
			k2_8 = 2, // FFX_BLUR_KERNEL_PERMUTATION_1
			k4_0 = 4, // FFX_BLUR_KERNEL_PERMUTATION_2
		};

		// values match FfxBlurKernelSize
		enum class EFFXBlurKernelSize
		{
			k3x3   = (1 << 0),
			k5x5   = (1 << 1),
			k7x7   = (1 << 2),
			k9x9   = (1 << 3),
			k11x11 = (1 << 4),
			k13x13 = (1 << 5),
			k15x15 = (1 << 6),
			k17x17 = (1 << 7),
			k19x19 = (1 << 8),
			k21x21 = (1 << 9),
		};

		void DispatchGaussianBlur(CommandBufferVulkan* CmdBuf, Texture2* Source, Texture2* Target, FidelityFXContext* Context, EFFXBlurSigma Sigma, EFFXBlurKernelSize KernelSize);

		RenderGraphTextureRef DispatchGaussianBlurRG(RenderGraph& Graph, RenderGraphTextureRef Source, FidelityFXContext* Context, EFFXBlurSigma Sigma, EFFXBlurKernelSize KernelSize);

		RenderGraphTextureRef DispatchDepthOfFieldRG(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, FidelityFXContext* Context);

		RenderGraphTextureRef DispatchReflectionsDenoiserRG(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, FidelityFXContext* Context);

		// TODO: shadow denoiser
		// TODO: reflections denoiser
		// TODO: SSR
	}

	// Anti-Aliasing
	namespace Antialiasing
	{
		void ApplyJitter(RenderView& View);

		RenderGraphTextureRef RenderTAA(RenderGraph& Graph, RenderView& View, SceneTextures& Textures);
	}

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History);
	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures);
	void PrepareTiledLights(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext);
	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures);
	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext);
	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef OverlayTexture); // only for DebugRender objects
	RenderGraphTextureRef DebugVisualisationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& HistoryTextures);
	void RenderUIPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef TextureToDrawTo);
	RenderGraphTextureRef RenderDeferred(RenderGraph& Graph, RenderView& View, DeferredRenderContext& HistoryTextures);

	void RenderPrepareSkyLut(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context);
	void RenderDeferredSky(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context, RenderGraphTextureRef OverTexture);
	RenderGraphTextureRef RenderVolumetricFog(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef SceneTexture);
	void RenderDeferredTransparency(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context);

	// Real-time raytracing
	//
	void RayTracedShadowsPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext);
	void RayTracedReflectionsPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext);
	void RayTracedGlobalIlluminationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext);

	// Light baking
	//
	// this pass renders a meshes in lightmap UV space which gives lightmap_texel-vertex correspondance
	// which is then used as sampling points for path tracing
	void PrepareAtlasForLightmapBaking(RenderGraph& Graph, LightmapSystem& System);
	void BakeLightmapPathTraced(RenderGraph& Graph, LightmapSystem& System);

	void RenderBakeIrradianceVolume(RenderGraph& Graph, const RenderView& View, GPUIrradianceVolume& Volume, int Spp, int Bounces);
	void RenderBakeReflectionCapture(RenderGraph& Graph, const RenderView& View, GPUReflectionCapture& Capture, int Spp, int Bounces);

	// Baked GI runtime
	//
	void RenderApplyIrradianceProbes(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext);

	// **********************************
	// Path-Tracing
	//
	struct PathTracingRenderContext
	{
		SPtr<DeviceVulkan> Device;
		Texture2* History = nullptr;
		u32 Frame = 0;
		Camera PreviousCamera;
		bool HasPreviousCamera = false;

		explicit PathTracingRenderContext(SPtr<DeviceVulkan> InDevice) : Device(InDevice) {}
		~PathTracingRenderContext();
		PathTracingRenderContext(const PathTracingRenderContext&) = delete;
		PathTracingRenderContext& operator=(const PathTracingRenderContext&) = delete;
	};

	RenderGraphTextureRef RenderPathTraced(RenderGraph& Graph, const RenderView& View, PathTracingRenderContext& Context);

}
