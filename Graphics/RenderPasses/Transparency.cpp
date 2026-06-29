#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Math/Quaternion.h"
#include <algorithm>
#include <array>
#include <vector>

DECLARE_GPU_PROFILING_COUNTER(GpuCounterTransparency);
DECLARE_GPU_PROFILING_COUNTER(GpuCounterTransparencyRefraction);

IMPLEMENT_GPU_PROFILING_COUNTER("Transparency", "RenderGraphGPU", GpuCounterTransparency);
IMPLEMENT_GPU_PROFILING_COUNTER("Transparency Refraction", "RenderGraphGPU", GpuCounterTransparencyRefraction);

namespace Columbus
{

	struct ParticlesParameters
	{
		Matrix ViewProjection = Matrix(1.0f);
		Matrix BillboardMatrix = Matrix(1.0f);
		iVector2 SubImages {};
		float SubImagesSpeed = 0.0f;
		float MotionVectorStrength = -1.0f; // < 0 if no motion vectors
	};

	struct TransparentMeshParameters
	{
		u32 ObjectId = 0;
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

	struct RefractionBlurShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RefractionBlur.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderSampledTexture Source;
			ShaderStaticSampler SourceSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderStorageTexture Output;
			ShaderConstants<RefractionBlurParameters> Constants;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Source, 0, 0);
			Binder.Bind(Params.SourceSampler, 0, 1);
			Binder.Bind(Params.Output, 0, 2);
			Binder.Bind(Params.Constants, 0, 3);
		}
	};

	struct ForwardTransparentShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/ForwardTransparent.csd";

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
			Desc.depthStencilState.DepthFunc = ComparisonFunc::GEqual;
			return Desc;
		}

		struct PassParameters
		{
			ShaderGPUScene Scene;
			ShaderSampledTexture RefractionLevel0;
			ShaderSampledTexture RefractionLevel1;
			ShaderSampledTexture RefractionLevel2;
			ShaderSampledTexture RefractionLevel3;
			ShaderStaticSampler RefractionSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
		};

		struct DrawParameters
		{
			ShaderPushConstants<TransparentMeshParameters> Object { {}, ShaderType::Vertex };
		};

		static void Bind(ShaderBinder& Binder, const PassParameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.RefractionLevel0, 2, 0);
			Binder.Bind(Params.RefractionLevel1, 2, 1);
			Binder.Bind(Params.RefractionLevel2, 2, 2);
			Binder.Bind(Params.RefractionLevel3, 2, 3);
			Binder.Bind(Params.RefractionSampler, 2, 4);
		}

		static void Bind(ShaderBinder& Binder, const DrawParameters& Params)
		{
			Binder.Bind(Params.Object);
		}
	};

	struct ParticlesShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/Particles.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			EParticleBlendMode Blend = EParticleBlendMode::Default;
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			RenderTargetBlendDesc BlendDesc;
			BlendDesc.BlendEnable = true;

			switch (Permutation.Blend)
			{
			case EParticleBlendMode::Add:
				BlendDesc.SrcBlend = Blend::One;
				BlendDesc.DestBlend = Blend::One;
				break;
			case EParticleBlendMode::Default:
			default:
				BlendDesc.SrcBlend = Blend::SrcAlpha;
				BlendDesc.DestBlend = Blend::InvSrcAlpha;
				break;
			}

			GraphicsPipelineDesc Desc;
			Desc.Name = "Particles";
			Desc.rasterizerState.Cull = CullMode::No;
			Desc.blendState.RenderTargets = { BlendDesc };
			Desc.depthStencilState.DepthEnable = true;
			Desc.depthStencilState.DepthWriteMask = false;
			Desc.depthStencilState.DepthFunc = ComparisonFunc::GEqual;
			return Desc;
		}

		struct Parameters
		{
			ShaderReadBuffer Particles;
			ShaderSampledTexture Texture;
			ShaderSampledTexture MotionVectors;
			ShaderStaticSampler Sampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ShaderPushConstants<ParticlesParameters> Constants { {}, ShaderType::Vertex | ShaderType::Pixel };
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.Particles, 0, 0);
			Binder.Bind(Params.Texture, 0, 1);
			Binder.Bind(Params.MotionVectors, 0, 2);
			Binder.Bind(Params.Sampler, 0, 3);
			Binder.Bind(Params.Constants);
		}
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

		RefractionBlurShader::Parameters BlurParams;
		BlurParams.Source = Source;
		BlurParams.Output = Target;
		BlurParams.Constants.Value.OutputSize = Size;
		BlurParams.Constants.Value.Radius = Radius;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RefractionBlurShader>(BlurParams);

		Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [Size, BlurParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparencyRefraction, Context);

			ComputePipeline* Pipeline = GetComputePipeline<RefractionBlurShader>(Context, RefractionBlurShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RefractionBlurShader>(Pipeline, BlurParams);
			Context.DispatchComputePixels(Pipeline, { 8, 8, 1 }, { Size, 1 });
		});

		return Target;
	}

	static void RenderTransparentMeshes(RenderGraphContext& Context, const RenderView& View, const ForwardTransparentShader::PassParameters& PassParams)
	{
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

		if (TransparentMeshes.empty())
			return;

		GraphicsPipeline* Pipeline = GetGraphicsPipeline<ForwardTransparentShader>(Context, ForwardTransparentShader::Permutation {}, ForwardTransparentShader::PipelinePermutation {});
		Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
		Context.BindGraphicsParameters<ForwardTransparentShader>(Pipeline, PassParams);

		for (const TransparentMeshDrawItem& DrawItem : TransparentMeshes)
		{
			const GPUSceneMesh& Mesh = Context.Scene->Meshes.Data()[DrawItem.ObjectId];

			ForwardTransparentShader::DrawParameters DrawParams;
			DrawParams.Object.Value.ObjectId = DrawItem.ObjectId;
			Context.BindGraphicsParameters<ForwardTransparentShader>(Pipeline, DrawParams);
			Context.CommandBuffer->Draw(Mesh.MeshResource->IndicesCount, 1, 0, 0);
		}
	}

	static Quaternion GetParticleBillboardRotation(const RenderView& View, EParticleBillboardMode Billboard)
	{
		switch (Billboard)
		{
		case EParticleBillboardMode::Vertical:
			return Quaternion(Vector3(0, View.CameraCur.Rot.Y, 0));
		case EParticleBillboardMode::Horizontal:
		case EParticleBillboardMode::FaceToCamera:
			return Quaternion(Vector3(-View.CameraCur.Rot.X, View.CameraCur.Rot.Y, 0));
		case EParticleBillboardMode::None:
		default:
			return Quaternion(Vector3(0, 0, 0));
		}
	}

	static void RenderParticles(RenderGraphContext& Context, const RenderView& View)
	{
		for (const auto& ParticleSystem : Context.Scene->Particles)
		{
			const u32 NumParticles = (u32)ParticleSystem.ParticleInstance->Particles.Count;
			if (NumParticles == 0 || ParticleSystem.DataBuffer == nullptr)
				continue;

			Texture2* ParticleTexture = ParticleSystem.ParticleInstance->Settings->Texture.Get();
			Texture2* MotionVectorsTexture = ParticleSystem.ParticleInstance->Settings->MotionVectors.Get();

			if (ParticleTexture == nullptr)
			{
				ParticleTexture = Context.Device->DefaultTextures.White;
			}

			if (MotionVectorsTexture == nullptr)
			{
				MotionVectorsTexture = Context.Device->DefaultTextures.Black;
			}

			ParticlesShader::Parameters Params;
			Params.Particles = ParticleSystem.DataBuffer;
			Params.Texture = ParticleTexture;
			Params.MotionVectors = MotionVectorsTexture;
			Params.Constants.Value.ViewProjection = View.CameraCur.GetViewProjection();
			Params.Constants.Value.BillboardMatrix = GetParticleBillboardRotation(View, ParticleSystem.ParticleInstance->Settings->Billboard).ToMatrix();
			Params.Constants.Value.SubImages = iVector2(ParticleSystem.ParticleInstance->Settings->SubUV.Horizontal, ParticleSystem.ParticleInstance->Settings->SubUV.Vertical);
			Params.Constants.Value.SubImagesSpeed = ParticleSystem.ParticleInstance->Settings->SubUV.Cycles;
			Params.Constants.Value.MotionVectorStrength = ParticleSystem.ParticleInstance->Settings->MotionVectors.Get()
				? ParticleSystem.ParticleInstance->Settings->MotionVectorStrength
				: -1.0f;

			ParticlesShader::PipelinePermutation PipelinePermutation {
				.Blend = ParticleSystem.ParticleInstance->Settings->Blend,
			};

			GraphicsPipeline* Pipeline = GetGraphicsPipeline<ParticlesShader>(Context, ParticlesShader::Permutation {}, PipelinePermutation);
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<ParticlesShader>(Pipeline, Params);
			Context.CommandBuffer->Draw(NumParticles * 6, 1, 0, 0);
		}
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
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 0.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		ForwardTransparentShader::PassParameters MeshPassParams;
		MeshPassParams.Scene.UseCombinedSampler = false;
		MeshPassParams.RefractionLevel0 = RefractionLevels[0];
		MeshPassParams.RefractionLevel1 = RefractionLevels[1];
		MeshPassParams.RefractionLevel2 = RefractionLevels[2];
		MeshPassParams.RefractionLevel3 = RefractionLevels[3];
		Dependencies.Bind<ForwardTransparentShader>(MeshPassParams);

		Graph.AddPass("Transparency", RenderGraphPassType::Raster, Parameters, Dependencies, [View, MeshPassParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparency, Context);
			RenderTransparentMeshes(Context, View, MeshPassParams);
			RenderParticles(Context, View);
		});
	}

}
