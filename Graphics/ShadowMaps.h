#pragma once

#include "Graphics/Core/ShadowAtlasAllocator.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include <unordered_map>

namespace Columbus
{
	inline constexpr u32 ShadowAtlasDescriptorCount = 64; // Mirrored in ShadowMaps.hlsli.
	inline constexpr u32 RTShadowMaskDescriptorCount = 1000; // Mirrored in GBufferLightingPass.hlsl.
	inline constexpr u32 ShadowTilePadding = 4;

	// Exactly mirrored in Shaders/ShadowMaps.hlsli. Projection matrices have no Vulkan Y flip.
	struct GPUShadowView
	{
		Matrix WorldToClip;
		Vector4 AtlasScaleBias;
		Vector4 SampleBounds;
		Vector4 Cascade; // start, end, blend start (positive camera-view depth), unused
		u32 AtlasIndex = ~0u; // Invalid means fully lit; no atlas sampling for skipped views.
		u32 _pad[3]{};
	};
	static_assert(sizeof(GPUShadowView) == 128);

	struct ShadowTile
	{
		u32 AtlasIndex;
		ShadowAtlasAllocator::Allocation Block;
	};

	struct ShadowDrawView
	{
		GPUShadowView GPU;
		ShadowTile Tile;
		float SlopeBias = 0;
		Matrix RasterWorldToClip;
		std::vector<u32> Casters; // Dense object IDs resolved only for views being redrawn.
		u64 Revision = 0;
		u64* RenderedRevision = nullptr;
		bool Render = true;
	};

	struct ShadowFrameData
	{
		std::vector<GPULightCompact> Lights;
		std::vector<ShadowDrawView> Views;
		std::vector<RenderGraphTextureRef> Atlases;
		RenderGraphBufferRef ViewBuffer = -1;
		u32 RTLightCount = 0;
	};

	struct RTShadowDenoiserHistory
	{
		Texture2* Shadow = nullptr;
		Texture2* Moments = nullptr;
		bool Valid = false; // Resources survive skipped frames; temporal contents do not.

		void Destroy(SPtr<DeviceVulkan> Device);
	};

	// Owned by a deferred context. Local shadow depth and placement survive frames.
	// RT history and allocations are keyed by sparse light handle + generation, never dense index.
	struct ShadowRenderState
	{
		struct AtlasPage
		{
			SPtr<Texture2> Texture;
			ShadowAtlasAllocator Allocator;
			explicit AtlasPage(u32 size) : Allocator(size) {}
		};
		struct LightAllocation
		{
			u32 Resolution;
			std::vector<ShadowTile> Tiles;
			std::vector<u64> RenderedRevisions;
		};

		std::vector<AtlasPage> Pages;
		std::unordered_map<u64, LightAllocation> Allocations;
		std::unordered_map<u64, RTShadowDenoiserHistory> RTHistories;
		ShadowFrameData Frame;
		u32 AtlasSize          = 0;
		u32 ViewBufferCapacity = 64; // GPUShadowView records; retain spare capacity between frames.
		bool CasterCulling = true;
		GPUScene* SceneIdentity = nullptr;

		void Destroy(SPtr<DeviceVulkan> device);
	};

	u32 ShadowCasterViewMask(const GPULight& Light, const Box& Bounds);
	u64 ShadowLightKey(const GPUScene& scene, u32 denseIndex);
	void PrepareShadows(RenderGraph& graph, const RenderView& view, ShadowRenderState& state);
	void RenderShadowMaps(RenderGraph& graph, const ShadowFrameData& frame);
}
