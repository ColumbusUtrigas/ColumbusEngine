#pragma once

#include "Graphics/ShaderBinder.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/Core/View.h"

#include <array>

namespace Columbus
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Baked lighting runtime sampling.
	//
	// Both prepare paths run once per view and their results are stored in
	// DeferredRenderContext::BakedLighting, where render passes read them. Prepared data holds
	// render-graph references that are only valid for the current frame's graph, so it is
	// refreshed every frame.

	namespace IrradianceVolumes
	{
		static constexpr int MaxVolumes = 16;

		struct VolumeDesc
		{
			Vector4 PositionIntensity;
			Vector4 ExtentNormalBias;
			Vector4 BlendPriority;
			iVector4 ProbesCountAndBufferIndex;
		};

		struct Constants
		{
			iVector4 CountAndFlags;
			VolumeDesc Volumes[MaxVolumes]{};
		};

		struct Prepared
		{
			ShaderConstants<Constants> Constants;
			std::array<ShaderReadBuffer, MaxVolumes> ProbeBuffers;
		};

		Prepared Prepare(RenderGraph& Graph, RenderGraphBufferRef FallbackBuffer, bool bEnabled);
	}

	namespace ReflectionCaptures
	{
		static constexpr int MaxCaptures = 32;

		struct CaptureDesc
		{
			Vector4 PositionIntensity;
			Vector4 ExtentBlendDistance; // xyz: full box size, w: blend distance.
			Vector4 PriorityMipEnabled;
		};

		struct Constants
		{
			iVector4 CountAndFlags;
			Vector4 Params;
			CaptureDesc Captures[MaxCaptures]{};
		};

		struct Prepared
		{
			ShaderConstants<Constants> Constants;
			std::array<ShaderSampledTexture, MaxCaptures> Radiance;
		};

		struct ShaderResources
		{
			ShaderConstants<Constants> Constants;
			ShaderSampledTexture BRDFLUT;
			ShaderStaticSampler Sampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderArray<ShaderSampledTexture> RadianceTextures;
		};

		Prepared Prepare(RenderGraph& Graph, const RenderView& View, bool bEnabled, bool bRTReflectionsAvailable = true);
		void SetShaderResources(ShaderResources& Resources, const Prepared& Captures, Texture2* BRDFLUT);
		void BindReflectionCaptureResources(ShaderBinder& Binder, const ShaderResources& Resources, int Set);
	}

	// per-view prepared baked lighting, lives in DeferredRenderContext
	struct BakedLightingPrepared
	{
		IrradianceVolumes::Prepared IrradianceVolumes;
		ReflectionCaptures::Prepared ReflectionCaptures;
	};
}
