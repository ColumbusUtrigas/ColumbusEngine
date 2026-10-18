#pragma once

#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/ShaderBinder.h"
#include <array>

namespace Columbus
{
	struct DeferredRenderContext;
	struct SceneTextures;
}

namespace Columbus::RadianceCache
{
	// Matches RadianceCacheConstants in RadianceCache.hlsli.
	struct Constants
	{
		Vector4  CameraPositionScale;
		Vector4  PreviousCameraPosition;
		iVector4 Config; // Entry count, frame index, accumulation frames, stale frames.
		iVector4 Update; // Update grid width/height, reserved, maximum path vertices.
		iVector4 Flags; // Gather statistics, sample reflections, sample GI, visualisation mode.
	};
	static_assert(sizeof(Constants) == 80);

	struct QueryParameters
	{
		bool Enabled = false;

		ShaderReadBuffer      HashEntries;
		ShaderReadBuffer      Accumulation;
		ShaderReadBuffer      Resolved;
		ShaderReadWriteBuffer Statistics;

		ShaderConstants<RadianceCache::Constants> Constants;

		void Bind(ShaderBinder& Binder, int Set = 3) const;
	};

	struct RadianceCacheData
	{
		QueryParameters Query;
		RenderGraphBufferRef DataBuffer = -1; // Valid fallback binding for baked probe arrays.
	};

	struct State
	{
		std::array<Buffer*, 4> Buffers{};

		u32 EntryCount      = 0;
		u32 FrameIndex      = 0;
		u32 ResetGeneration = 0;

		Vector3 PreviousCameraPosition{};
		float SceneScale = 0;
		bool ActiveLastFrame = false;

		void Destroy(SPtr<DeviceVulkan> Device);
	};

	// Up to one update path per 8x8 input region, capped by the path budget.
	iVector2 CalculateUpdateGrid(iVector2 RenderSize, u32 PathBudget);

	bool IsReflectionCacheRequested(const RenderView& View);

	void Prepare(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context);
	RenderGraphTextureRef VisualiseRadianceCache(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures);
}
