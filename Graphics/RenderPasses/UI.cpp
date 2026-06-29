#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"

namespace Columbus
{

	struct UIPerDrawcallParams
	{
		Vector4 Colour{};
		Vector2 Position{};
		Vector2 Size{};
	};

	struct UIShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/UI.csd";

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
			Desc.Name = "UI";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = {
				RenderTargetBlendDesc {
					.BlendEnable = true,
					.SrcBlend = Blend::SrcAlpha,
					.DestBlend = Blend::InvSrcAlpha,
				}
			};
			Desc.depthStencilState.DepthEnable = false;
			Desc.depthStencilState.DepthWriteMask = false;
			return Desc;
		}

		struct Parameters
		{
			ShaderSampledTexture Texture;
			ShaderStaticSampler Sampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<UIPerDrawcallParams> Constants { {}, ShaderType::Vertex | ShaderType::Pixel };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Texture,   0, 0);
			Binder.Bind(Params.Sampler,   0, 1);
			Binder.Bind(Params.Constants);
		}
	};

	void RenderUIPass(RenderGraph& Graph, RenderView& View, RenderGraphTextureRef TextureToDrawTo)
	{
		if (View.UI == nullptr)
			return;

		RenderPassParameters Parameters;
		Parameters.ViewportSize = View.OutputSize;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, TextureToDrawTo, {} };

		for (const UIImage* Img : View.UI->Images)
		{
			UIShader::Parameters DrawParams;
			DrawParams.Texture = Img->Img;
			Dependencies.Bind<UIShader>(DrawParams);
		}

		Graph.AddPass("UI", RenderGraphPassType::Raster, Parameters, Dependencies, [View, TextureToDrawTo](RenderGraphContext& Context)
		{
			GraphicsPipeline* Pipeline = GetGraphicsPipeline<UIShader>(Context, UIShader::Permutation {}, UIShader::PipelinePermutation {});

			for (const UIImage* Img : View.UI->Images)
			{
				UIShader::Parameters DrawParams;
				DrawParams.Texture = Img->Img;
				DrawParams.Constants.Value.Colour = Img->Colour;
				DrawParams.Constants.Value.Position = Img->Position;
				DrawParams.Constants.Value.Size = Img->GetActualRenderSize(View.CameraCur.GetAspect());

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
				Context.BindGraphicsParameters<UIShader>(Pipeline, DrawParams);
				Context.CommandBuffer->Draw(6, 1, 0, 0);
			}
		});
	}

}
