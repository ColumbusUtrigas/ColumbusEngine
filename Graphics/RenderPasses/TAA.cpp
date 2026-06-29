#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"

namespace Columbus::Antialiasing
{

	DECLARE_GPU_PROFILING_COUNTER(GpuCounterTAA);

	IMPLEMENT_GPU_PROFILING_COUNTER("TAA", "RenderGraphGPU", GpuCounterTAA);

	struct TAAConstants
	{
		iVector2 Resolution;
	};

	struct TAAShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/TAA.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture Image;
			ShaderSampledTexture History;
			ShaderSampledTexture VelocityImage;
			ShaderStorageTexture Output;
			ShaderStaticSampler LinearSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderConstants<TAAConstants> Constants;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Image,         0, 0);
			Binder.Bind(Params.History,       0, 1);
			Binder.Bind(Params.VelocityImage, 0, 2);
			Binder.Bind(Params.Output,        0, 3);
			Binder.Bind(Params.LinearSampler, 0, 4);
			Binder.Bind(Params.Constants,     0, 5);
		}
	};

	static float Halton(int32_t index, int32_t base)
	{
		float f = 1.0f, result = 0.0f;

		for (int32_t currentIndex = index; currentIndex > 0;)
		{
			f /= (float)base;
			result = result + f * (float)(currentIndex % base);
			currentIndex = (uint32_t)(floorf((float)(currentIndex) / (float)(base)));
		}

		return result;
	}

	void ApplyJitter(RenderView& View)
	{
		const int SequenceLength = 10;

		int Index = (GFrameNumber % SequenceLength) + 1;
		float x = (Halton(Index, 2) - 0.5f) / (float)View.RenderSize.X;
		float y = (Halton(Index, 3) - 0.5f) / (float)View.RenderSize.Y;

		View.CameraCur.ApplyProjectionJitter(x, y);
	}

	RenderGraphTextureRef RenderTAA(RenderGraph& Graph, RenderView& View, SceneTextures& Textures)
	{
		TextureDesc2 Desc = Graph.GetTextureDesc(Textures.FinalBeforeTonemap);
		Desc.Usage = TextureUsage::StorageSampled;
		Graph.CreateHistoryTexture(&Textures.History.TAAHistory, Desc, "FinalHistory");

		iVector2 Size = View.RenderSize;

		RenderGraphTextureRef OutputTexture = Graph.CreateTexture(Desc, "FinalTAA");

		TAAShader::Parameters TAAParams;
		TAAParams.Image         = Textures.FinalBeforeTonemap;
		TAAParams.History       = Textures.History.TAAHistory;
		TAAParams.VelocityImage = Textures.Velocity;
		TAAParams.Output        = OutputTexture;
		TAAParams.Constants.Value.Resolution = Size;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<TAAShader>(TAAParams);

		Graph.AddPass("TAA", RenderGraphPassType::Compute, Parameters, Dependencies, [Size, TAAParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTAA, Context);

			ComputePipeline* Pipeline = GetComputePipeline<TAAShader>(Context, TAAShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<TAAShader>(Pipeline, TAAParams);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});

		Graph.ExtractTexture(OutputTexture, &Textures.History.TAAHistory);

		return OutputTexture;
	}

}
