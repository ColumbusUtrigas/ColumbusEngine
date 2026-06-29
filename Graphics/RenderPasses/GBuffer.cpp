#include "Common/Image/Image.h"
#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/Pipelines.h"
#include "Graphics/Core/Texture.h"
#include "Graphics/Core/Types.h"
#include "Graphics/Core/View.h"
#include "Graphics/GPUScene.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/ShaderCache.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Profiling/Profiling.h"
#include "RayTracingIrradianceVolumes.h"
#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Columbus
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterGBufferPass);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterGBufferDecals);
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterLightingPass);

	IMPLEMENT_GPU_PROFILING_COUNTER("GBuffer", "RenderGraphGPU", GpuCounterGBufferPass);
	IMPLEMENT_GPU_PROFILING_COUNTER("GBuffer Decals", "RenderGraphGPU", GpuCounterGBufferDecals);
	IMPLEMENT_GPU_PROFILING_COUNTER("Lighting pass", "RenderGraphGPU", GpuCounterLightingPass);

	extern ConsoleVariable<bool> CVar_RayTracingIrradianceVolumes;

	struct PerObjectParameters
	{
		u32 ObjectId;
	};

	static bool IsTransparentMesh(const SPtr<GPUScene>& Scene, const GPUSceneMesh& Mesh)
	{
		const Material* Mat = Scene->Materials.Get(Mesh.MaterialId);
		return Mat != nullptr && (Mat->ShadingMode == MaterialShadingMode::Transparent || Mat->ShadingMode == MaterialShadingMode::Refractive);
	}

	struct PerDecalParameters
	{
		Matrix Model, ModelInverse;
		Matrix VP;
	};

	struct GBufferBasePassShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/GBufferPass.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "GBufferPass";
			Desc.rasterizerState.Cull = CullMode::Front;
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
			};
			return Desc;
		}

		struct Parameters
		{
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
		}
	}; // GBufferBasePassShader

	struct DecalShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/Decals.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "Decals";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc {
					.BlendEnable = true,
					.SrcBlend = Blend::SrcAlpha,
					.DestBlend = Blend::InvSrcAlpha,
				},
			};
			Desc.depthStencilState.DepthEnable = false;
			Desc.depthStencilState.DepthWriteMask = false;
			return Desc;
		}

		struct PassParameters
		{
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderReadBuffer SceneBuffer;
		};

		struct DrawParameters
		{
			ShaderSampledTexture DecalTexture;
			ShaderStaticSampler DecalSampler;
		};

		static void Bind(ShaderBinder& Binder, const PassParameters& Params)
		{
			Binder.Bind(Params.GBufferDepth, 0, 0);
			Binder.Bind(Params.SceneBuffer, 0, 1);
		}

		static void Bind(ShaderBinder& Binder, const DrawParameters& Params)
		{
			Binder.Bind(Params.DecalTexture, 1, 0);
			Binder.Bind(Params.DecalSampler, 1, 1);
		}
	}; // DecalShader

	struct DeferredLightingShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/GBufferLightingPass.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture GBufferAlbedo;
			ShaderSampledTexture GBufferNormal;
			ShaderSampledTexture GBufferDepth { TextureBindingFlags::AspectDepth };
			ShaderSampledTexture GBufferRoughnessMetallic;
			ShaderSampledTexture GBufferEmissive;
			ShaderSampledTexture GBufferLightmap;
			ShaderSampledTexture GBufferReflections;
			ShaderSampledTexture GBufferGI;
			ShaderStorageTexture LightingOutput;

			ShaderReadBuffer LightsBuffer;
			ShaderReadBuffer SceneBuffer;

			ShaderSampledTexture LTC1;
			ShaderSampledTexture LTC2;
			ShaderStaticSampler LTCSampler;

			ShaderArray<ShaderStorageTexture> ShadowTextures;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.GBufferAlbedo, 0, 0);
			Binder.Bind(Params.GBufferNormal, 0, 1);
			Binder.Bind(Params.GBufferDepth, 0, 2);
			Binder.Bind(Params.GBufferRoughnessMetallic, 0, 3);
			Binder.Bind(Params.GBufferEmissive, 0, 4);
			Binder.Bind(Params.GBufferLightmap, 0, 5);
			Binder.Bind(Params.GBufferReflections, 0, 6);
			Binder.Bind(Params.GBufferGI, 0, 7);
			Binder.Bind(Params.LightingOutput, 0, 8);
			Binder.Bind(Params.LightsBuffer, 0, 9);
			Binder.Bind(Params.SceneBuffer, 0, 10);
			Binder.Bind(Params.LTC1, 0, 11);
			Binder.Bind(Params.LTC2, 0, 12);
			Binder.Bind(Params.LTCSampler, 0, 13);
			Binder.Bind(Params.ShadowTextures, 1, 0);
		}
	}; // DeferredLightingShader

	SceneTextures CreateSceneTextures(RenderGraph& Graph, const RenderView& View, HistorySceneTextures& History)
	{
		TextureDesc2 CommonDesc;
		CommonDesc.Usage     = TextureUsage::RenderTargetColor;
		CommonDesc.Width     = View.RenderSize.X;
		CommonDesc.Height    = View.RenderSize.Y;
		CommonDesc.AddressU  = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressV  = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressW  = TextureAddressMode::ClampToEdge;
		CommonDesc.MagFilter = TextureFilter2::Nearest;
		CommonDesc.MinFilter = TextureFilter2::Nearest;
		CommonDesc.MipFilter = TextureFilter2::Nearest;

		TextureDesc2 AlbedoDesc   = CommonDesc;
		TextureDesc2 NormalDesc   = CommonDesc;
		TextureDesc2 RMDesc       = CommonDesc;
		TextureDesc2 EmissiveDesc = CommonDesc;
		TextureDesc2 DSDesc       = CommonDesc;
		TextureDesc2 VelocityDesc = CommonDesc;
		TextureDesc2 LightmapDesc = CommonDesc;
		AlbedoDesc.Format   = TextureFormat::RGBA8;
		NormalDesc.Format   = TextureFormat::RG16F;
		RMDesc.Format       = TextureFormat::RG8;
		EmissiveDesc.Format = TextureFormat::R11G11B10F;
		DSDesc.Format       = TextureFormat::Depth32F;
		DSDesc.Usage        = TextureUsage::RenderTargetDepth;
		VelocityDesc.Format = TextureFormat::RG16F;
		LightmapDesc.Format = TextureFormat::R11G11B10F;
		
		SceneTextures Result { .History = History };
		Result.GBufferAlbedo   = Graph.CreateTexture(AlbedoDesc, "GBufferAlbedo");
		Result.GBufferNormal   = Graph.CreateTexture(NormalDesc, "GBufferNormal");
		Result.GBufferRM       = Graph.CreateTexture(RMDesc, "GBufferRM");
		Result.GBufferEmissive = Graph.CreateTexture(EmissiveDesc, "GBufferEmissive");
		Result.GBufferDS       = Graph.CreateTexture(DSDesc, "GBufferDS");
		Result.Velocity        = Graph.CreateTexture(VelocityDesc, "Velocity");
		Result.Lightmap        = Graph.CreateTexture(LightmapDesc, "Lightmap");
		Result.VolumetricFog   = Result.GBufferAlbedo;

		// history textures
		Graph.CreateHistoryTexture(&Result.History.Depth, DSDesc, "GBufferDS History");
		Graph.CreateHistoryTexture(&Result.History.Normals, NormalDesc, "GBufferNormal History");
		Graph.CreateHistoryTexture(&Result.History.RoughnessMetallic, RMDesc, "GBufferRM History");

		return Result;
	}

	void ExtractHistorySceneTextures(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, HistorySceneTextures& HistoryTextures)
	{
		Graph.ExtractTexture(Textures.GBufferDS, &HistoryTextures.Depth);
		Graph.ExtractTexture(Textures.GBufferRM, &HistoryTextures.RoughnessMetallic);
		Graph.ExtractTexture(Textures.GBufferNormal, &HistoryTextures.Normals);
	}

	void HistorySceneTextures::Destroy(SPtr<DeviceVulkan> Device)
	{
		Device->DestroyTexture(Depth);
		Device->DestroyTexture(RoughnessMetallic);
		Device->DestroyTexture(Normals);
		Device->DestroyTexture(VolumetricFogFroxels);
		Device->DestroyTexture(TAAHistory);
		Device->DestroyTexture(RTReflectionsRadiance);
		Device->DestroyTexture(RTReflectionsAverageRadiance);
		Device->DestroyTexture(RTReflectionsVariance);
		Device->DestroyTexture(RTReflectionsSampleCount);
		RTGI_History.Destroy(Device);
		Depth = nullptr;
		RoughnessMetallic = nullptr;
		Normals = nullptr;
		VolumetricFogFroxels = nullptr;
		TAAHistory = nullptr;
		RTReflectionsRadiance = nullptr;
		RTReflectionsAverageRadiance = nullptr;
		RTReflectionsVariance = nullptr;
		RTReflectionsSampleCount = nullptr;
	}

	void RenderGBufferPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferAlbedo };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferNormal };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferRM };
		Parameters.ColorAttachments[3] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferEmissive };
		Parameters.ColorAttachments[4] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.Velocity };
		Parameters.ColorAttachments[5] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.Lightmap };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.GBufferDS, AttachmentClearValue{ {}, 0.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		Graph.AddPass("GBufferBasePass", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterGBufferPass, Context);

			GBufferBasePassShader::Parameters GBufferParams;
			GraphicsPipeline* Pipeline = GetGraphicsPipeline<GBufferBasePassShader>(Context, GBufferBasePassShader::Permutation {}, GBufferBasePassShader::PipelinePermutation {});
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<GBufferBasePassShader>(Pipeline, GBufferParams);
			Context.BindGPUScene(Pipeline, false);

			for (int i = 0; i < Context.Scene->Meshes.Size(); i++)
			{
				const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];
				if (IsTransparentMesh(Context.Scene, Mesh))
					continue;

				PerObjectParameters Parameters;
				Parameters.ObjectId = i;

				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
			}
		});
	}

	void RenderGBufferDecals(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferAlbedo };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		DecalShader::PassParameters DecalPassParams;
		DecalPassParams.GBufferDepth = Textures.GBufferDS;
		DecalPassParams.SceneBuffer = Graph.Scene->SceneBuffer;
		Dependencies.Bind<DecalShader>(DecalPassParams);

		Graph.AddPass("GBufferDecals", RenderGraphPassType::Raster, Parameters, Dependencies, [View, DecalPassParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterGBufferDecals, Context);

			GraphicsPipeline* Pipeline = GetGraphicsPipeline<DecalShader>(Context, DecalShader::Permutation {}, DecalShader::PipelinePermutation {});
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<DecalShader>(Pipeline, DecalPassParams);

			PerDecalParameters Parameters;

			for (int i = 0; i < Context.Scene->Decals.Size(); i++)
			{
				GPUDecal& Decal = ((GPUDecal*)Context.Scene->Decals.Data())[i]; // TODO: nasty
				Parameters.Model = Decal.Model;
				Parameters.ModelInverse = Decal.ModelInverse;
				Parameters.VP = View.CameraCur.GetViewProjection();

				if (!Decal.Texture)
					continue;

				DecalShader::DrawParameters DecalParams;
				DecalParams.DecalTexture = Decal.Texture;
				Context.BindGraphicsParameters<DecalShader>(Pipeline, DecalParams);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
				Context.CommandBuffer->Draw(36, 1, 0, 0);
			}
		});
	}

	RenderGraphTextureRef RenderDeferredLightingPass(RenderGraph& Graph, const RenderView& View, const SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		TextureDesc2 Desc {
			.Usage = TextureUsage::StorageSampled | TextureUsage::RenderTargetColor,
			.Width = (uint32)View.RenderSize.X,
			.Height = (uint32)View.RenderSize.Y,
			.Format = TextureFormat::RGBA16F,
		};

		static const iVector2 GroupSize { 8, 8 };

		RenderGraphTextureRef LightingTexture = Graph.CreateTexture(Desc, "Lighting");

		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);

		DeferredLightingShader::Parameters LightingParams;
		LightingParams.GBufferAlbedo            = Textures.GBufferAlbedo;
		LightingParams.GBufferNormal            = Textures.GBufferNormal;
		LightingParams.GBufferDepth             = Textures.GBufferDS;
		LightingParams.GBufferRoughnessMetallic = Textures.GBufferRM;
		LightingParams.GBufferEmissive          = Textures.GBufferEmissive;
		LightingParams.GBufferLightmap          = Textures.Lightmap;
		LightingParams.GBufferReflections       = Textures.RTReflections;
		LightingParams.GBufferGI                = Textures.RTGI;
		LightingParams.LightingOutput           = LightingTexture;
		LightingParams.LightsBuffer             = Graph.Scene->LightsBuffer;
		LightingParams.SceneBuffer              = Graph.Scene->SceneBuffer;
		LightingParams.LTC1                     = Graph.Scene->LTC_1;
		LightingParams.LTC2                     = Graph.Scene->LTC_2;
		std::vector<ShaderStorageTexture> ShadowTextures;
		ShadowTextures.reserve(DeferredContext.LightRenderInfos.size());

		for (GPULightRenderInfo& LightInfo : DeferredContext.LightRenderInfos)
		{
			ShaderStorageTexture ShadowTexture;
			ShadowTexture = LightInfo.RTShadow;
			ShadowTextures.push_back(ShadowTexture);
		}
		LightingParams.ShadowTextures.Set(ShadowTextures.data(), (uint32_t)ShadowTextures.size());

		Dependencies.Bind<DeferredLightingShader>(LightingParams);

		Graph.AddPass("DeferredLightingPass", RenderGraphPassType::Compute, Parameters, Dependencies, [View, LightingParams, ShadowTextures](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterLightingPass, Context);

			ComputePipeline* Pipeline = GetComputePipeline<DeferredLightingShader>(Context, DeferredLightingShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			DeferredLightingShader::Parameters RuntimeParams = LightingParams;
			RuntimeParams.ShadowTextures.Set(ShadowTextures.data(), (uint32_t)ShadowTextures.size());
			Context.BindComputeParameters<DeferredLightingShader>(Pipeline, RuntimeParams);

			const iVector2 GroupCount = (View.RenderSize + (GroupSize - 1)) / GroupSize;
			Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
		});

		return LightingTexture;
	}

	struct MemsetTextureParameters
	{
		Vector4  Value;
		iVector2 TextureSize;
	};

	void ShaderMemsetTexture(RenderGraph& Graph, RenderGraphTextureRef Texture, Vector4 Value)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Write(Texture, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass("MemsetTexture", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture, Value](RenderGraphContext& Context)
		{
			SPtr<Texture2> Tex = Context.GetRenderGraphTexture(Texture);
			Context.CommandBuffer->MemsetTexture(Tex.get(), Value);
		});
	}

	// TODO: find a way to do it without this pass
	static void TransitionImagePass(RenderGraph& Graph, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Texture, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		Graph.AddPass("TransitionImage", RenderGraphPassType::Compute, Parameters, Dependencies, [Texture](RenderGraphContext& Context)
		{
			Context.CommandBuffer->TransitionImageLayout(Context.GetRenderGraphTexture(Texture).get(), VK_IMAGE_LAYOUT_GENERAL);
		});
	}

	void PrepareDeferredView(RenderView& View)
	{
		View.CameraCurUnjittered = View.CameraCur;
		View.CameraPrevUnjittered = View.CameraPrev;

		if (View.DeferredSettings.ApplyFSR1)
		{
			View.RenderSize = iVector2((int)(View.OutputSize.X * View.DeferredSettings.RenderResolution), (int)(View.OutputSize.Y * View.DeferredSettings.RenderResolution));
		}
		else
		{
			View.RenderSize = View.OutputSize;
		}

		if (View.DeferredSettings.ApplyTAA)
		{
			Antialiasing::ApplyJitter(View);
		}
	}

	RenderGraphTextureRef RenderDeferred(RenderGraph& Graph, RenderView& View, DeferredRenderContext& DeferredContext)
	{
		DeferredContext.VisualisationMode = View.DeferredSettings.VisualisationMode;

		SceneTextures Textures = CreateSceneTextures(Graph, View, DeferredContext.History);

		DeferredContext.LightRenderInfos.clear();

		RenderPrepareSkyLut(Graph, View, Textures, DeferredContext);
		PrepareTiledLights(Graph, View);
		RenderGBufferPass(Graph, View, Textures);
		RenderGBufferDecals(Graph, View, Textures);

		// TODO: only if it's needed
		RadianceCache::TraceRadianceCache(Graph, View, Textures.RadianceCache);

		// TODO: use dummy structured buffer instead
		RayTracingIrradianceVolumes::Prepared IrradianceVolumes = RayTracingIrradianceVolumes::Prepare(Graph, Textures.RadianceCache.DataBuffer, CVar_RayTracingIrradianceVolumes.GetValue());

		RayTracedShadowsPass(Graph, View, Textures, DeferredContext);
		RayTracedReflectionsPass(Graph, View, Textures, DeferredContext, IrradianceVolumes);

		switch (View.DeferredSettings.GlobalIlluminationMode)
		{
		case EDeferredGlobalIlluminationMode::RTGI:
			RayTracedGlobalIlluminationPass(Graph, View, Textures, DeferredContext, IrradianceVolumes);
			break;

		case EDeferredGlobalIlluminationMode::IV:
			RenderApplyIrradianceProbes(Graph, View, Textures, IrradianceVolumes);
			break;
		case EDeferredGlobalIlluminationMode::None:
		{
			TextureDesc2 Desc{
				.Usage = TextureUsage::Storage | TextureUsage::Sampled,
				.Width = (u32)View.RenderSize.X,
				.Height = (u32)View.RenderSize.Y,
				.Format = TextureFormat::RGBA16F,
			};

			Textures.RTGI = Graph.CreateTexture(Desc, "EmptyGI");
			ShaderMemsetTexture(Graph, Textures.RTGI, {});
		}
		}

		Textures.FinalBeforeTonemap = RenderDeferredLightingPass(Graph, View, Textures, DeferredContext);
		RenderDeferredSky(Graph, View, Textures, DeferredContext, Textures.FinalBeforeTonemap);
		Textures.FinalBeforeTonemap = RenderVolumetricFog(Graph, View, Textures, Textures.FinalBeforeTonemap);
		RenderDeferredTransparency(Graph, View, Textures, DeferredContext);

		// TODO: find a way to apply DoF after TAA
		Textures.FinalBeforeTonemap = FFX::DispatchDepthOfFieldRG(Graph, View, Textures, DeferredContext.FFX);

		if (View.DeferredSettings.ApplyTAA)
		{
			Textures.FinalBeforeTonemap = Antialiasing::RenderTAA(Graph, View, Textures);
		}

		RenderGraphTextureRef TonemappedImage = TonemapPass(Graph, View, Textures.FinalBeforeTonemap);
		Textures.FinalAfterTonemap = TonemappedImage;
		ScreenshotPass(Graph, View, View.ScreenshotHDR ? Textures.FinalBeforeTonemap : TonemappedImage);

		DebugOverlayPass(Graph, View, Textures, TonemappedImage);

		// debug visualisation modes
		if (DeferredContext.VisualisationMode != EDeferredRenderVisualisationMode::Final)
		{
			if (DeferredContext.VisualisationMode == EDeferredRenderVisualisationMode::RadianceCache)
			{
				TonemappedImage = RadianceCache::VisualiseRadianceCache(Graph, View, Textures.RadianceCache, Textures.GBufferDS);
			}
			else
			{
				TonemappedImage = DebugVisualisationPass(Graph, View, Textures, DeferredContext);
			}
		}

		if (View.DeferredSettings.ApplyFSR1)
		{
			TextureDesc2 FsrUpscaleDesc = Graph.GetTextureDesc(TonemappedImage);
			//FsrUpscaleDesc.Usage = TextureUsage::StorageSampled;
			FsrUpscaleDesc.Width = View.OutputSize.X;
			FsrUpscaleDesc.Height = View.OutputSize.Y;

			const iVector2 UpscaleTo = View.OutputSize;
			const float UpscaleFactor = 1.0f / View.DeferredSettings.RenderResolution;
			const bool IsHdr = true;

			TonemappedImage = ApplyFSR1(Graph, TonemappedImage, FsrUpscaleDesc, UpscaleTo, IsHdr, View.DeferredSettings.ApplyFSR1Sharpening, View.DeferredSettings.FSR1Sharpening);
		}

		RenderUIPass(Graph, View, TonemappedImage);

		TransitionImagePass(Graph, TonemappedImage);
		ExtractHistorySceneTextures(Graph, View, Textures, DeferredContext.History);

		return TonemappedImage;
	}

}
