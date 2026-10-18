#include "RenderPasses.h"
#include "Graphics/ShaderCache.h"
#include "ShaderBytecode/ShaderBytecode.h"
#include "Math/Quaternion.h"
#include <algorithm>
#include <cstdio>

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
	};
	static_assert(sizeof(RefractionBlurParameters) == 8);

	struct RefractionBlurShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/RefractionBlur.csd";

		struct Permutation
		{
			int Kernel = 0; // Copy, radius 2, radius 4, radius 8.
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
			Builder.Axis("BLUR_KERNEL", &Permutation::Kernel);
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
			ShaderSampledTexture RefractionPyramid;
			ShaderStaticSampler RefractionSampler { SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
			ReflectionCaptures::ShaderResources ReflectionCaptures;
		};

		struct DrawParameters
		{
			ShaderPushConstants<TransparentMeshParameters> Object { {}, ShaderType::Vertex };
		};

		static void Bind(ShaderBinder& Binder, const PassParameters& Params)
		{
			Binder.Bind(Params.Scene);
			Binder.Bind(Params.RefractionPyramid, 2, 0);
			Binder.Bind(Params.RefractionSampler, 2, 1);
			ReflectionCaptures::BindReflectionCaptureResources(Binder, Params.ReflectionCaptures, 3);
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

	static void AddRefractionBlurPass(RenderGraph& Graph, RenderGraphTextureViewRef Source, RenderGraphTextureViewRef Output, const char* Name, int Kernel)
	{
		const iVector2 Size = Graph.GetTextureSize2D(Output);

		RefractionBlurShader::Parameters BlurParams;
		BlurParams.Source = Source;
		BlurParams.Output = Output;
		BlurParams.Constants.Value.OutputSize = Size;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<RefractionBlurShader>(BlurParams);

		Graph.AddPass(Name, RenderGraphPassType::Compute, Parameters, Dependencies, [Size, Kernel, BlurParams](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparencyRefraction, Context);

			ComputePipeline* Pipeline = GetComputePipeline<RefractionBlurShader>(Context, { Kernel });
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<RefractionBlurShader>(Pipeline, BlurParams);

			// Blur groups produce an 8x32 tile with 8x8 threads; the sharp copy produces 8x8.
			const iVector3 TileSize = Kernel == 0 ? iVector3(8, 8, 1) : iVector3(8, 32, 1);
			Context.DispatchComputePixels(Pipeline, TileSize, { Size, 1 });
		});
	}

	static void RenderTransparentMeshes(RenderGraphContext& Context, const RenderView& View, const ForwardTransparentShader::PassParameters& PassParams)
	{
		RenderGraphDynamicArray<TransparentMeshDrawItem> TransparentMeshes(Context.Graph.Allocator);
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
		TextureDesc2 PyramidDesc = Graph.GetTextureDesc(Textures.FinalBeforeTonemap);
		PyramidDesc.Format = TextureFormat::RGBA16F;
		PyramidDesc.Mips   = 1;

		for (u32 Dimension = std::max(PyramidDesc.Width, PyramidDesc.Height); Dimension > 1 && PyramidDesc.Mips < 4; Dimension >>= 1)
		{
			++PyramidDesc.Mips;
		}

		const auto Pyramid = Graph.CreateTexture(PyramidDesc, "Refraction Pyramid");
		auto Source = Graph.CreateTextureView(Textures.FinalBeforeTonemap, { .MipCount = 1 });

		for (u32 Mip = 0; Mip < PyramidDesc.Mips; ++Mip)
		{
			const auto Output = Graph.CreateTextureView(Pyramid, { .BaseMip = Mip, .MipCount = 1 });
			char Name[32];
			snprintf(Name, sizeof(Name), "Refraction Level %u", Mip);
			AddRefractionBlurPass(Graph, Source, Output, Name, int(Mip));
			Source = Output;
		}

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.FinalBeforeTonemap };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 0.0f, 0 } };
		Parameters.ViewportSize = View.RenderSize;

		RenderPassDependencies Dependencies(Graph.Allocator);

		struct TransparencyPassData
		{
			ReflectionCaptures::Prepared ReflectionCaptures;
			ForwardTransparentShader::PassParameters MeshPassParams;
		};

		TransparencyPassData PassData;
		PassData.ReflectionCaptures = Context.BakedLighting.ReflectionCaptures;
		PassData.MeshPassParams.Scene.UseCombinedSampler = false;
		PassData.MeshPassParams.RefractionPyramid = Pyramid;
		ReflectionCaptures::SetShaderResources(PassData.MeshPassParams.ReflectionCaptures, PassData.ReflectionCaptures, Graph.Scene->BRDF_LUT);
		Dependencies.Bind<ForwardTransparentShader>(PassData.MeshPassParams);

		Graph.AddPass("Transparency", RenderGraphPassType::Raster, Parameters, Dependencies, [View, PassData](RenderGraphContext& Context) mutable
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTransparency, Context);
			ReflectionCaptures::SetShaderResources(PassData.MeshPassParams.ReflectionCaptures, PassData.ReflectionCaptures, Context.Scene->BRDF_LUT);
			RenderTransparentMeshes(Context, View, PassData.MeshPassParams);
			RenderParticles(Context, View);
		});
	}

}
