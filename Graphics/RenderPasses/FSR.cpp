#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"

namespace Columbus
{

	struct FSR1Parameters
	{
		iVector2 ImageSize;
		iVector2 MaxImageSize;
		iVector2 OutputSize;
		float Sharpening;
		int IsSharpeningPass;
		int IsHdr;
	};

	struct FSR1PSShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/FSR1/FSR1PS.csd";

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
			Desc.Name = "FSR1 (VS/PS)";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
			return Desc;
		}

		struct Parameters
		{
			ShaderSampledTexture Input;
			ShaderStaticSampler InputSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<FSR1Parameters> Constants { {}, ShaderType::Vertex | ShaderType::Pixel };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Input, 0, 0);
			Binder.Bind(Params.InputSampler, 0, 1);
			Binder.Bind(Params.Constants);
		}
	};

	struct FSR1CSShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/FSR1/FSR1CS.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture Input;
			ShaderStaticSampler InputSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderStorageTexture Output;
			ShaderPushConstants<FSR1Parameters> Constants { {}, ShaderType::Compute };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Input, 0, 0);
			Binder.Bind(Params.InputSampler, 0, 1);
			Binder.Bind(Params.Output, 0, 2);
			Binder.Bind(Params.Constants);
		}
	};

	// TODO: support HDR input (apply tonemapping and inverse tonemapping)
	// TODO: manage descriptor sets properly (if it's used multiple times in the frame)
	// TODO: adding those screen-size and compute renderpasses seems repetative, create a common util for that task (requires a shader system)
	RenderGraphTextureRef ApplyFSR1(RenderGraph& Graph, RenderGraphTextureRef Texture, const TextureDesc2& DstDesc, iVector2 UpscaleTo, bool IsHdr, bool UseSharpening, float Sharpening)
	{
		COLUMBUS_ASSERT(DstDesc.Usage != TextureUsage::Sampled);
		COLUMBUS_ASSERT(DstDesc.Usage != TextureUsage::RenderTargetDepth);

		TextureDesc2 Desc = Graph.GetTextureDesc(Texture);

		iVector2 Size = iVector2(Desc.Width, Desc.Height);
		iVector2 DstSize = iVector2(DstDesc.Width, DstDesc.Height);
		iVector2 UpscaleSize = UpscaleTo;

		const char* TexName = Graph.GetTextureName(Texture);
		char ResultName[512]{ 0 };
		snprintf(ResultName, 512, "%s (FSR1 upscaled)", TexName);

		char SharpenedResultName[512]{ 0 };
		snprintf(SharpenedResultName, 512, "%s (FSR upscaled + sharpened)", TexName);

		RenderGraphTextureRef Result = Graph.CreateTexture(DstDesc, ResultName);

		const bool bUsePS = DstDesc.Usage == TextureUsage::RenderTargetColor;

		FSR1Parameters FsrParameters{
			.ImageSize = Size,
			.MaxImageSize = DstSize,
			.OutputSize = UpscaleSize,
			.Sharpening = Sharpening,
			.IsSharpeningPass = 0,
			.IsHdr = IsHdr ? 1 : 0,
		};

		// FSR1 EASU upscale pass
		if (bUsePS) // vertex/pixel shader path
		{
			RenderPassParameters Parameters;
			Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Result,  };
			Parameters.ViewportSize = UpscaleSize;

			RenderPassDependencies Dependencies(Graph.Allocator);

			char PassName[256]{ 0 };
			snprintf(PassName, 256, "FSR1 (PS) %ix%i->%ix%i", Size.X, Size.Y, UpscaleSize.X, UpscaleSize.Y);

			FSR1PSShader::Parameters ShaderParams;
			ShaderParams.Input = Texture;
			ShaderParams.Constants.Value = FsrParameters;
			Dependencies.Bind<FSR1PSShader>(ShaderParams);

			Graph.AddPass(PassName, RenderGraphPassType::Raster, Parameters, Dependencies, [ShaderParams](RenderGraphContext& Context)
			{
				GraphicsPipeline* Pipeline = GetGraphicsPipeline<FSR1PSShader>(Context, FSR1PSShader::Permutation {}, FSR1PSShader::PipelinePermutation {});

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
				Context.BindGraphicsParameters<FSR1PSShader>(Pipeline, ShaderParams);
				Context.CommandBuffer->Draw(3, 1, 0, 0);
			});
		}
		else // compute shader path
		{
			RenderPassParameters Parameters;
			RenderPassDependencies Dependencies(Graph.Allocator);

			char PassName[256]{ 0 };
			snprintf(PassName, 256, "FSR1 (CS) %ix%i->%ix%i", Size.X, Size.Y, UpscaleSize.X, UpscaleSize.Y);

			FSR1CSShader::Parameters ShaderParams;
			ShaderParams.Input = Texture;
			ShaderParams.Output = Result;
			ShaderParams.Constants.Value = FsrParameters;
			Dependencies.Bind<FSR1CSShader>(ShaderParams);

			Graph.AddPass(PassName, RenderGraphPassType::Compute, Parameters, Dependencies, [UpscaleSize, ShaderParams](RenderGraphContext& Context)
			{
				ComputePipeline* Pipeline = GetComputePipeline<FSR1CSShader>(Context, FSR1CSShader::Permutation {});

				const int GroupSize = 16; // actual group size is 64x1, which is 8x8, but FSR works in 2x2 blocks
				const iVector2 GroupCount = (UpscaleSize + (GroupSize - 1)) / GroupSize;

				Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<FSR1CSShader>(Pipeline, ShaderParams);
				Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
			});
		}

		// FSR1 RCAS sharpening pass
		if (UseSharpening)
		{
			FsrParameters.IsSharpeningPass = 1;

			RenderGraphTextureRef SharpeningResult = Graph.CreateTexture(DstDesc, SharpenedResultName);

			if (bUsePS)
			{
				RenderPassParameters Parameters;
				Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, SharpeningResult, };
				Parameters.ViewportSize = DstSize;

				RenderPassDependencies Dependencies(Graph.Allocator);

				char PassName[256]{ 0 };
				snprintf(PassName, 256, "FSR1 (PS) sharpening (%.2f)", Sharpening);

				FSR1PSShader::Parameters ShaderParams;
				ShaderParams.Input = Result;
				ShaderParams.Constants.Value = FsrParameters;
				Dependencies.Bind<FSR1PSShader>(ShaderParams);

				Graph.AddPass(PassName, RenderGraphPassType::Raster, Parameters, Dependencies, [ShaderParams](RenderGraphContext& Context)
				{
					GraphicsPipeline* Pipeline = GetGraphicsPipeline<FSR1PSShader>(Context, FSR1PSShader::Permutation {}, FSR1PSShader::PipelinePermutation {});

					Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
					Context.BindGraphicsParameters<FSR1PSShader>(Pipeline, ShaderParams);
					Context.CommandBuffer->Draw(3, 1, 0, 0);
				});
			}

			Result = SharpeningResult;
		}

		return Result;
	}

}
