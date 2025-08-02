#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Math/Quaternion.h"

DECLARE_GPU_PROFILING_COUNTER(GpuCounterTransparency);

IMPLEMENT_GPU_PROFILING_COUNTER("Transparency", "RenderGraphGPU", GpuCounterTransparency);

namespace Columbus
{

	struct ParticlesParameters
	{
		Matrix ViewProjection;
		Matrix BillboardMatrix;
		iVector2 Frame;
	};

	void RenderDeferredTransparency(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.FinalBeforeTonemap };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		Graph.AddPass("Transparency", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparency, Context);

			static GraphicsPipeline* Pipelines[2]{ nullptr };
			if (Pipelines[0] == nullptr)
			{
				RenderTargetBlendDesc BlendVariation[2];

				// EParticleBlendMode::Default
				BlendVariation[0] = RenderTargetBlendDesc{
					.BlendEnable = true,
					.SrcBlend = Blend::SrcAlpha,
					.DestBlend = Blend::InvSrcAlpha,
				};

				// EParticleBlendMode::Add
				BlendVariation[1] = RenderTargetBlendDesc{
					.BlendEnable = true,
					.SrcBlend = Blend::One,
					.DestBlend = Blend::One,
				};

				for (int i = 0; i < 2; i++)
				{
					GraphicsPipelineDesc Desc;
					Desc.Name = "Particles";
					Desc.rasterizerState.Cull = CullMode::No;

					Desc.blendState.RenderTargets = {
						BlendVariation[i]
					};

					Desc.depthStencilState.DepthEnable = true;
					Desc.depthStencilState.DepthWriteMask = false;
					Desc.depthStencilState.DepthFunc = ComparisonFunc::LEqual;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Particles.csd");

					Pipelines[i] = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
				}
			}

			for (const auto& ParticleSystem : Context.Scene->Particles)
			{
				GraphicsPipeline* Pipeline = Pipelines[(u32)ParticleSystem.ParticleInstance->Settings->Blend];

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);

				u32 NumParticles = (u32)ParticleSystem.ParticleInstance->Particles.Count;

				if (NumParticles == 0)
					continue;

				if (ParticleSystem.DataBuffer == nullptr)
					continue;

				Quaternion Q;

				switch (ParticleSystem.ParticleInstance->Settings->Billboard)
				{
				case EParticleBillboardMode::Vertical:
					Q = Quaternion(Vector3(0, View.CameraCur.Rot.Y, 0));
					break;
				case EParticleBillboardMode::Horizontal:
				case EParticleBillboardMode::FaceToCamera:
					Q = Quaternion(Vector3(-View.CameraCur.Rot.X, View.CameraCur.Rot.Y, 0));
					break;
				case EParticleBillboardMode::None:
				default:
				Q = Quaternion(Vector3(0,0,0));
				}

				ParticlesParameters Params;
				Params.ViewProjection = View.CameraCur.GetViewProjection();
				Params.BillboardMatrix = Q.ToMatrix();
				Params.Frame = iVector2(ParticleSystem.ParticleInstance->Settings->SubUV.Horizontal, ParticleSystem.ParticleInstance->Settings->SubUV.Vertical);

				Texture2* ParticleTexture = ParticleSystem.ParticleInstance->Settings->Texture.Asset;

				if (ParticleTexture == nullptr)
				{
					ParticleTexture = Context.Device->DefaultTextures.White;
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, ParticleSystem.DataBuffer);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, ParticleTexture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.Device->GetStaticSampler());

				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Draw(NumParticles*6, 1, 0, 0);
			}
		});
	}

}