#include "Graphics/Lightmaps.h"
#include "Graphics/ShaderCache.h"
#include "RenderPasses.h"

namespace Columbus
{

	struct LightmapRasterisationParameters
	{
		u64 VertexBuffer;
		u64 IndexBuffer;
		u32 ObjectId;
	};

	struct LightmapPathTracingParameters
	{
		int Random;
		int Bounces;
		int AccumulatedSamples;
		int RequestedSamples;
		int SamplesPerFrame;
	};

	struct LightmapRasterisationShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/LightmapRasterisation.csd";

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
			Desc.Name = "LightmapRasterisation";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
				RenderTargetBlendDesc(),
			};
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
		};

		struct DrawParameters
		{
			ShaderPushConstants<LightmapRasterisationParameters> Constants { {}, ShaderType::Vertex | ShaderType::Pixel };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
		}

		static void Bind(ShaderBinder& Binder, const DrawParameters& Params)
		{
			Binder.Bind(Params.Constants);
		}
	};

	struct LightmapPathTracingShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/LightmapPathTracing.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			u32 MaxRecursionDepth = 1;
		};

		static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RayTracingPipelineDesc Desc {};
			Desc.Name = "LightmapPathTracing";
			Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
			return Desc;
		}

		struct Parameters
		{
			ShaderGPUScene Scene;
			ShaderAccelerationStructure AccelerationStructure;
			ShaderSampledTexture PositionsImage;
			ShaderSampledTexture NormalsImage;
			ShaderSampledTexture ValidityImage;
			ShaderStorageTexture LightmapOutput;
			ShaderPushConstants<LightmapPathTracingParameters> Constants { {}, ShaderType::Raygen };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.AccelerationStructure, 2, 0);
			Binder.Bind(Params.PositionsImage, 2, 1);
			Binder.Bind(Params.NormalsImage, 2, 2);
			Binder.Bind(Params.ValidityImage, 2, 3);
			Binder.Bind(Params.LightmapOutput, 2, 4);
			Binder.Bind(Params.Constants);
		}
	};

	// see note in the declaration
	void PrepareAtlasForLightmapBaking(RenderGraph& Graph, LightmapSystem& System)
	{
		LightmapBakingSettings& Settings = System.BakingSettings;
		LightmapBakingRenderData& Data = System.BakingData;

		// TODO: unify format creation with GBuffer?
		TextureDesc2 CommonDesc;
		CommonDesc.Width = System.Atlas.Width;
		CommonDesc.Height = System.Atlas.Height;
		CommonDesc.Type = TextureType::Texture2D;
		CommonDesc.Usage = TextureUsage::RenderTargetColor;
		CommonDesc.AddressU = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressV = TextureAddressMode::ClampToEdge;
		CommonDesc.AddressW = TextureAddressMode::ClampToEdge;
		CommonDesc.MagFilter = TextureFilter2::Nearest;
		CommonDesc.MinFilter = TextureFilter2::Nearest;
		CommonDesc.MipFilter = TextureFilter2::Nearest;

		TextureDesc2 NormalDesc = CommonDesc;
		TextureDesc2 PositionDesc = CommonDesc;
		TextureDesc2 ValidityDesc = CommonDesc;
		NormalDesc.Format = TextureFormat::RGBA16F;
		PositionDesc.Format = TextureFormat::RGBA32F;
		ValidityDesc.Format = TextureFormat::R8;

		System.BakingData.Normal = Graph.CreateTexture(NormalDesc, "LightmapBaking_Normal");
		System.BakingData.Position = Graph.CreateTexture(PositionDesc, "LightmapBaking_Posiiton");
		System.BakingData.Validity = Graph.CreateTexture(ValidityDesc, "LightmapBaking_Validity");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Normal };
		Parameters.ColorAttachments[1] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Position };
		Parameters.ColorAttachments[2] = RenderPassAttachment{ AttachmentLoadOp::Clear, System.BakingData.Validity };
		Parameters.ViewportSize = iVector2(System.Atlas.Width, System.Atlas.Height);

		iVector2 ViewportSize = Parameters.ViewportSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		LightmapRasterisationShader::Parameters RasterParams;
		Graph.AddPass("PrepareMeshForLightmapBaking", RenderGraphPassType::Raster, Parameters, Dependencies, [&System, ViewportSize, RasterParams](RenderGraphContext& Context)
		{
			GraphicsPipeline* Pipeline = GetGraphicsPipeline<LightmapRasterisationShader>(Context, LightmapRasterisationShader::Permutation {}, LightmapRasterisationShader::PipelinePermutation {});
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<LightmapRasterisationShader>(Pipeline, RasterParams);

			for (u32 i = 0; i < System.Meshes.size(); i++)
			{
				const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];

				LightmapRasterisationShader::DrawParameters DrawParams;
				DrawParams.Constants.Value = {
					.VertexBuffer = System.Meshes[i].VertexBuffer->GetDeviceAddress(),
					.IndexBuffer = System.Meshes[i].IndexBuffer->GetDeviceAddress(),
					.ObjectId = i,
				};

				Context.BindGraphicsParameters<LightmapRasterisationShader>(Pipeline, DrawParams);
				Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
			}
		});
	}

	void BakeLightmapPathTraced(RenderGraph& Graph, LightmapSystem& System)
	{
		LightmapBakingRenderData& Data = System.BakingData;
		LightmapBakingSettings& Settings = System.BakingSettings;

		if (Data.AccumulatedSamples >= Settings.RequestedSamples)
		{
			System.BakingRequested = false;
			return;
		}

		RenderPassParameters Parameters;

		LightmapPathTracingShader::Parameters TraceParams;
		TraceParams.Scene.UseCombinedSampler = false;
		TraceParams.PositionsImage = Data.Position;
		TraceParams.NormalsImage = Data.Normal;
		TraceParams.ValidityImage = Data.Validity;
		TraceParams.LightmapOutput = System.Atlas.Lightmap;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<LightmapPathTracingShader>(TraceParams);

		int TraceSamples = Math::Min(Settings.SamplesPerFrame, Settings.RequestedSamples - Data.AccumulatedSamples);

		u32 Width = System.Atlas.Width;
		u32 Height = System.Atlas.Height;

		Graph.AddPass("LightmapPathTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [TraceParams, Data, Settings, TraceSamples, Width, Height](RenderGraphContext& Context)
		{
			RayTracingPipeline* Pipeline = GetRayTracingPipeline<LightmapPathTracingShader>(Context, LightmapPathTracingShader::Permutation {}, LightmapPathTracingShader::PipelinePermutation {});

			LightmapPathTracingShader::Parameters Parameters = TraceParams;
			Parameters.AccelerationStructure = Context.Scene->TLAS;
			Parameters.Constants.Value = {
				.Random = rand(),
				.Bounces = Settings.Bounces,
				.AccumulatedSamples = Data.AccumulatedSamples,
				.RequestedSamples = Settings.RequestedSamples,
				.SamplesPerFrame = TraceSamples,
			};

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.BindRayTracingParameters<LightmapPathTracingShader>(Pipeline, Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, Width, Height, 1);
		});

		Data.AccumulatedSamples += TraceSamples;
	}

}
