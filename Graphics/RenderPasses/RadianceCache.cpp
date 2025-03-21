#include "RadianceCache.h"

namespace Columbus::RadianceCache
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterTraceRadianceCache);

	IMPLEMENT_GPU_PROFILING_COUNTER("RadianceCache", "RenderGraphGPU", GpuCounterTraceRadianceCache);

	static constexpr int NumCascades = 6;
	static constexpr int CascadeResolution = 16;

	struct RadianceCacheEntry
	{
		Vector4 Irradiance; // just plain omnidirectional irradiance for now
	};

	struct RadianceCacheTracingParams
	{
		Vector3 CameraPosition;
		int     Random;
	};

	void TraceRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache)
	{
		const int Resolution = CascadeResolution * CascadeResolution * CascadeResolution;
		BufferDesc Desc(sizeof(RadianceCacheEntry) *  Resolution * NumCascades, BufferType::UAV);
		RadianceCache.DataBuffer = Graph.CreateBuffer(Desc, "RadianceCache");

		// 2. run tracing shader
		RenderPassParameters Parameters;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.WriteBuffer(RadianceCache.DataBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

		Graph.AddPass("RadianceCacheTrace", RenderGraphPassType::Compute, Parameters, Dependencies, [View, RadianceCache](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTraceRadianceCache, Context);

			static RayTracingPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				RayTracingPipelineDesc Desc{};
				Desc.Name = "RadianceCacheTrace";
				Desc.MaxRecursionDepth = 1;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RadianceCache/RadianceCacheTrace.csd");
				Pipeline = Context.Device->CreateRayTracingPipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 2);

			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.Scene->TLAS);
			Context.Device->UpdateDescriptorSet(Set, 1, 0, Context.GetRenderGraphBuffer(RadianceCache.DataBuffer).get());

			Context.CommandBuffer->BindRayTracingPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &Set);
			Context.BindGPUScene(Pipeline, false);

			RadianceCacheTracingParams Parameters{
				.CameraPosition = View.CameraCur.Pos,
				.Random = rand(),
			};

			Context.CommandBuffer->PushConstantsRayTracing(Pipeline, ShaderType::Raygen, 0, sizeof(Parameters), &Parameters);
			Context.CommandBuffer->TraceRays(Pipeline, NumCascades, CascadeResolution*CascadeResolution*CascadeResolution, 1);
		});
	}

	struct RadianceCacheVisualiseParams
	{
		Vector3 CameraPosition;
	};

	RenderGraphTextureRef VisualiseRadianceCache(RenderGraph& Graph, RenderView& View, RadianceCacheData& RadianceCache, RenderGraphTextureRef GBufferWP)
	{
		iVector2 Size = View.RenderSize;

		TextureDesc2 ResultDesc;
		ResultDesc.Format = TextureFormat::R11G11B10F; // format to get less banding
		ResultDesc.Usage = TextureUsage::RenderTargetColor;
		ResultDesc.Width = (u32)Size.X;
		ResultDesc.Height = (u32)Size.Y;

		RenderGraphTextureRef Target = Graph.CreateTexture(ResultDesc, "VisualisationResult");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Target, {} };
		Parameters.ViewportSize = Size;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.ReadBuffer(RadianceCache.DataBuffer, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		Dependencies.Read(GBufferWP, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		Vector3 CameraPos = View.CameraCur.Pos;

		Graph.AddPass("Radiance Cache Visualisation", RenderGraphPassType::Raster, Parameters, Dependencies, [CameraPos, Size, GBufferWP, RadianceCache](RenderGraphContext& Context)
		{
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "RadianceCacheVis";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc(),
				};

				Desc.depthStencilState.DepthEnable = false;
				Desc.depthStencilState.DepthWriteMask = false;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RadianceCache/RadianceCacheVisualise.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			RadianceCacheVisualiseParams Params
			{
				.CameraPosition = CameraPos
			};

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphBuffer(RadianceCache.DataBuffer).get());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.GetRenderGraphTexture(GBufferWP).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);

			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});

		return Target;
	}

}
