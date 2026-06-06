#include "RenderPasses.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Math/Quaternion.h"
#include <algorithm>
#include <array>
#include <vector>

DECLARE_GPU_PROFILING_COUNTER(GpuCounterTransparency);

IMPLEMENT_GPU_PROFILING_COUNTER("Transparency", "RenderGraphGPU", GpuCounterTransparency);

namespace Columbus
{

	struct ParticlesParameters
	{
		Matrix ViewProjection;
		Matrix BillboardMatrix;
		iVector2 SubImages;
		float SubImagesSpeed;
		float MotionVectorStrength; // < 0 if no motion vectors
	};

	struct TransparentMeshParameters
	{
		u32 ObjectId;
	};

	struct TransparentMeshDrawItem
	{
		u32 ObjectId = 0;
		float DistanceSq = 0.0f;
	};

	struct RefractionBlurParameters
	{
		iVector2 OutputSize;
		int Radius;
		float _pad;
	};

	static bool IsTransparentMesh(const SPtr<GPUScene>& Scene, const GPUSceneMesh& Mesh)
	{
		const Material* Mat = Scene->Materials.Get(Mesh.MaterialId);
		return Mat != nullptr && (Mat->ShadingMode == MaterialShadingMode::Transparent || Mat->ShadingMode == MaterialShadingMode::Refractive);
	}

	static float DistanceSquared(const Vector3& A, const Vector3& B)
	{
		const Vector3 D = A - B;
		return D.X * D.X + D.Y * D.Y + D.Z * D.Z;
	}

	static RenderGraphTextureRef AddRefractionBlurPass(RenderGraph& Graph, RenderGraphTextureRef Source, const char* Name, int Radius, int Downsample)
	{
		TextureDesc2 Desc = Graph.GetTextureDesc(Source);
		Desc.Format = TextureFormat::RGBA16F;
		Desc.Width = std::max(1u, Desc.Width / (u32)std::max(Downsample, 1));
		Desc.Height = std::max(1u, Desc.Height / (u32)std::max(Downsample, 1));

		RenderGraphTextureRef Target = Graph.CreateTexture(Desc, Name);
		iVector2 Size = Graph.GetTextureSize2D(Target);

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(Source, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(Target, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [Source, Target, Size, Radius](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/RefractionBlur.csd");
				Desc.Name = "RefractionBlur";

				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Source).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.Device->GetStaticSampler());
			Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, Context.GetRenderGraphTexture(Target).get());

			RefractionBlurParameters Params{
				.OutputSize = Size,
				.Radius = Radius,
			};

			const int GroupSize = 8;
			const iVector2 GroupCount = (Size + (GroupSize - 1)) / GroupSize;

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &DescriptorSet);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.CommandBuffer->Dispatch((u32)GroupCount.X, (u32)GroupCount.Y, 1);
		});

		return Target;
	}

	void RenderDeferredTransparency(RenderGraph& Graph, RenderView& View, SceneTextures& Textures, DeferredRenderContext& Context)
	{
		std::array<RenderGraphTextureRef, 4> RefractionLevels;
		RefractionLevels[0] = AddRefractionBlurPass(Graph, Textures.FinalBeforeTonemap, "Refraction Level 0", 0, 1);
		RefractionLevels[1] = AddRefractionBlurPass(Graph, RefractionLevels[0], "Refraction Level 1", 2, 2);
		RefractionLevels[2] = AddRefractionBlurPass(Graph, RefractionLevels[1], "Refraction Level 2", 4, 2);
		RefractionLevels[3] = AddRefractionBlurPass(Graph, RefractionLevels[2], "Refraction Level 3", 8, 2);

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.FinalBeforeTonemap };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);
		for (RenderGraphTextureRef RefractionLevel : RefractionLevels)
		{
			Dependencies.Read(RefractionLevel, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}

		Graph.AddPass("Transparency", RenderGraphPassType::Raster, Parameters, Dependencies, [View, RefractionLevels](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparency, Context);

			static GraphicsPipeline* Pipelines[2]{ nullptr };
			static GraphicsPipeline* TransparentMeshPipeline = nullptr;
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

			if (TransparentMeshPipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "ForwardTransparent";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc{
						.BlendEnable = true,
						.SrcBlend = Blend::SrcAlpha,
						.DestBlend = Blend::InvSrcAlpha,
					},
				};
				Desc.depthStencilState.DepthEnable = true;
				Desc.depthStencilState.DepthWriteMask = false;
				Desc.depthStencilState.DepthFunc = ComparisonFunc::LEqual;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/ForwardTransparent.csd");

				TransparentMeshPipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			std::vector<TransparentMeshDrawItem> TransparentMeshes;
			TransparentMeshes.reserve(Context.Scene->Meshes.Size());
			for (int i = 0; i < Context.Scene->Meshes.Size(); i++)
			{
				const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[i];
				if (!IsTransparentMesh(Context.Scene, Mesh))
					continue;

				TransparentMeshes.push_back(TransparentMeshDrawItem{
					.ObjectId = (u32)i,
					.DistanceSq = DistanceSquared(Mesh.Transform.ExtractTranslate(), View.CameraCur.Pos),
				});
			}

			std::sort(TransparentMeshes.begin(), TransparentMeshes.end(), [](const TransparentMeshDrawItem& A, const TransparentMeshDrawItem& B)
			{
				return A.DistanceSq > B.DistanceSq;
			});

			if (!TransparentMeshes.empty())
			{
				Context.CommandBuffer->BindGraphicsPipeline(TransparentMeshPipeline);
				Context.BindGPUScene(TransparentMeshPipeline, false);

				auto RefractionSet = Context.GetDescriptorSet(TransparentMeshPipeline, 2);
				Context.Device->UpdateDescriptorSet(RefractionSet, 0, 0, Context.GetRenderGraphTexture(RefractionLevels[0]).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(RefractionSet, 1, 0, Context.GetRenderGraphTexture(RefractionLevels[1]).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(RefractionSet, 2, 0, Context.GetRenderGraphTexture(RefractionLevels[2]).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(RefractionSet, 3, 0, Context.GetRenderGraphTexture(RefractionLevels[3]).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(RefractionSet, 4, 0, Context.Device->GetStaticSampler());
				Context.CommandBuffer->BindDescriptorSetsGraphics(TransparentMeshPipeline, 2, 1, &RefractionSet);

				for (const TransparentMeshDrawItem& DrawItem : TransparentMeshes)
				{
					const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[DrawItem.ObjectId];

					TransparentMeshParameters Params{ .ObjectId = DrawItem.ObjectId };
					Context.CommandBuffer->PushConstantsGraphics(TransparentMeshPipeline, ShaderType::Vertex, 0, sizeof(Params), &Params);
					Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
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
				Params.SubImages = iVector2(ParticleSystem.ParticleInstance->Settings->SubUV.Horizontal, ParticleSystem.ParticleInstance->Settings->SubUV.Vertical);
				Params.SubImagesSpeed = ParticleSystem.ParticleInstance->Settings->SubUV.Cycles;

				Texture2* ParticleTexture = ParticleSystem.ParticleInstance->Settings->Texture.Get();
				Texture2* MotionVectorsTexture = ParticleSystem.ParticleInstance->Settings->MotionVectors.Get();

				Params.MotionVectorStrength = MotionVectorsTexture ? ParticleSystem.ParticleInstance->Settings->MotionVectorStrength : -1.0f;

				if (ParticleTexture == nullptr)
				{
					ParticleTexture = Context.Device->DefaultTextures.White;
				}

				if (MotionVectorsTexture == nullptr)
				{
					MotionVectorsTexture = Context.Device->DefaultTextures.Black;
				}

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, ParticleSystem.DataBuffer);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, ParticleTexture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 2, 0, MotionVectorsTexture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 3, 0, Context.Device->GetStaticSampler());

				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Params), &Params);
				Context.CommandBuffer->Draw(NumParticles*6, 1, 0, 0);
			}
		});
	}

}
