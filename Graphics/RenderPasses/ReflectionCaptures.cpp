#include "RenderPasses.h"

#include "Core/CVar.h"
#include "Graphics/BakedLighting.h"
#include "Graphics/ShaderCache.h"

#include <vector>

namespace Columbus
{
	DECLARE_GPU_PROFILING_COUNTER(GpuCounterReflectionCaptureBake);
	IMPLEMENT_GPU_PROFILING_COUNTER("Reflection Capture Bake", "RenderGraphGPU", GpuCounterReflectionCaptureBake);

	ConsoleVariable<int> CVar_ReflectionCaptureBakeSampleChunk("r.ReflectionCaptures.BakeSampleChunk", "Maximum samples per texel processed by one reflection capture bake pass", 128);

	namespace
	{
		// mirrored in Shaders/ReflectionCaptures/ReflectionCaptureBake.hlsl
		struct ReflectionCaptureBakeParameters
		{
			Vector4 CapturePosition; // xyz: world position, w: unused

			int Resolution;
			int SampleOffset;
			int SamplesThisPass;
			int TotalSamples;

			int Bounces;
			int _Pad0;
			int _Pad1;
			int _Pad2;
		};

		// mirrored in Shaders/ReflectionCaptures/ReflectionCaptureConvolve.hlsl
		struct ReflectionCaptureConvolveParameters
		{
			int TargetMipSize;
			int TargetMip;
			int MipCount;
			int SampleOffset;

			int SamplesThisPass;
			int TotalSamples;
			int _Pad0;
			int _Pad1;
		};

		struct ReflectionCaptureBakeShader
		{
			static constexpr const char* Path = "./PrecompiledShaders/ReflectionCaptures/ReflectionCaptureBake.csd";

			struct Permutation {};
			static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>&) {}

			struct PipelinePermutation
			{
				u32 MaxRecursionDepth = 1;
			};

			static RayTracingPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
			{
				RayTracingPipelineDesc Desc{};
				Desc.Name = "ReflectionCaptureBake";
				Desc.MaxRecursionDepth = Permutation.MaxRecursionDepth;
				return Desc;
			}

			struct Parameters
			{
				ShaderGPUScene Scene;
				ShaderAccelerationStructure AccelerationStructure;
				ShaderStorageTexture RadianceMip0;
				ShaderPushConstants<ReflectionCaptureBakeParameters> Constants{ {}, ShaderType::Raygen };
			};

			static void Bind(ShaderBinder& Binder, const Parameters& Params)
			{
				Binder.Bind(Params.Scene);
				Binder.Bind(Params.AccelerationStructure, 2, 0);
				Binder.Bind(Params.RadianceMip0, 2, 1);
				Binder.Bind(Params.Constants);
			}
		};

		struct ReflectionCaptureConvolveShader
		{
			static constexpr const char* Path = "./PrecompiledShaders/ReflectionCaptures/ReflectionCaptureConvolve.csd";

			struct Permutation {};
			static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>&) {}

			struct Parameters
			{
				ShaderSampledTexture SourceRadiance;
				ShaderStaticSampler SourceSampler{ SamplerDesc::Create<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>() };
				ShaderStorageTexture TargetMip;
				ShaderPushConstants<ReflectionCaptureConvolveParameters> Constants{ {}, ShaderType::Compute };
			};

			static void Bind(ShaderBinder& Binder, const Parameters& Params)
			{
				Binder.Bind(Params.SourceRadiance, 0, 0);
				Binder.Bind(Params.SourceSampler, 0, 1);
				Binder.Bind(Params.TargetMip, 0, 2);
				Binder.Bind(Params.Constants);
			}
		};
	}

	void RenderBakeReflectionCapture(RenderGraph& Graph, const RenderView& View, GPUReflectionCapture& Capture, int Spp, int Bounces)
	{
		(void)View;

		if (Graph.Scene == nullptr || !Graph.Device->SupportsRayTracing())
		{
			Log::Warning("Reflection capture bake skipped: ray tracing is not available");
			return;
		}

		Capture.Resolution = BakedLighting::ClampReflectionCaptureResolution(Capture.Resolution > 0 ? Capture.Resolution : 128);
		const int Resolution = Capture.Resolution;
		const int SamplesPerTexel = Math::Clamp(Spp, 1, 4096);
		const int BounceCount = Math::Clamp(Bounces, 1, 8);

		Texture2* RadianceTexture = Graph.Scene->EnsureReflectionBakeCubemap(Capture);
		if (RadianceTexture == nullptr)
		{
			Log::Error("Reflection capture bake skipped: failed to create bake cubemap (%i)", Resolution);
			return;
		}

		const int MipCount = Capture.MipCount;

		// GPUScene owns the texture and outlives the graph; the graph only borrows it
		RenderGraphTextureRef Radiance = Graph.RegisterExternalTexture(SPtr<Texture2>(RadianceTexture, [](Texture2*) {}), "ReflectionCaptureRadiance");

		// per-mip storage views over the cube faces; destroyed after the graph is done with them
		std::vector<Texture2*> MipViews;
		MipViews.reserve((size_t)MipCount);
		for (u32 Mip = 0; Mip < (u32)MipCount; Mip++)
		{
			TextureViewDesc ViewDesc;
			ViewDesc.ViewType = TextureType::Texture2DArray;
			ViewDesc.BaseMip = Mip;
			ViewDesc.MipCount = 1;
			ViewDesc.BaseLayer = 0;
			ViewDesc.LayerCount = 6;
			MipViews.push_back(Graph.Device->CreateTextureView(RadianceTexture, ViewDesc));
		}

		const int SampleChunk = Math::Max(1, CVar_ReflectionCaptureBakeSampleChunk.GetValue());
		const int SamplePassCount = (SamplesPerTexel + SampleChunk - 1) / SampleChunk;
		Log::Message("Reflection capture bake scheduled: %i resolution, %i mips, %i spp, %i sample chunk, %i graph passes",
			Resolution, MipCount, SamplesPerTexel, SampleChunk, SamplePassCount * MipCount);

		// bring the (possibly freshly created) cubemap into the layout the graph keeps images in
		{
			RenderPassParameters PassParameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Read(Radiance, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			Graph.AddPass("ReflectionCaptureBakePrepare", RenderGraphPassType::Compute, PassParameters, Dependencies, [Radiance](RenderGraphContext& Context)
			{
				Context.CommandBuffer->TransitionImageLayout(Context.GetRenderGraphTexture(Radiance).get(), VK_IMAGE_LAYOUT_GENERAL);
			});
		}

		// mip 0: ray traced source radiance, accumulated over sample chunks.
		// each chunk reads the previous accumulation, so passes declare read+write.
		for (int SampleOffset = 0; SampleOffset < SamplesPerTexel; SampleOffset += SampleChunk)
		{
			ReflectionCaptureBakeShader::Parameters BakeParams;
			BakeParams.Scene.UseCombinedSampler = false;
			BakeParams.RadianceMip0 = MipViews[0];
			BakeParams.Constants.Value = {
				.CapturePosition = Vector4(Capture.Position, 0.0f),
				.Resolution = Resolution,
				.SampleOffset = SampleOffset,
				.SamplesThisPass = Math::Min(SampleChunk, SamplesPerTexel - SampleOffset),
				.TotalSamples = SamplesPerTexel,
				.Bounces = BounceCount,
			};

			RenderPassParameters PassParameters;
			RenderPassDependencies Dependencies(Graph.Allocator);
			Dependencies.Bind<ReflectionCaptureBakeShader>(BakeParams);
			Dependencies.Read(Radiance, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);
			Dependencies.Write(Radiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR);

			Graph.AddPass("ReflectionCaptureBake", RenderGraphPassType::Compute, PassParameters, Dependencies, [BakeParams, Resolution](RenderGraphContext& Context)
			{
				RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterReflectionCaptureBake, Context);

				ReflectionCaptureBakeShader::Parameters RuntimeParams = BakeParams;
				RuntimeParams.AccelerationStructure = Context.Scene->TLAS;

				RayTracingPipeline* Pipeline = GetRayTracingPipeline<ReflectionCaptureBakeShader>(Context, ReflectionCaptureBakeShader::Permutation{}, ReflectionCaptureBakeShader::PipelinePermutation{});
				Context.BindRayTracingPipeline<ReflectionCaptureBakeShader>(Pipeline, RuntimeParams);
				Context.CommandBuffer->TraceRays(Pipeline, (u32)Resolution, (u32)(6 * Resolution), 1);
			});
		}

		// mips 1..N-1: GGX convolution of mip 0, accumulated over sample chunks
		for (int Mip = 1; Mip < MipCount; Mip++)
		{
			const int MipSize = Math::Max(1, Resolution >> Mip);

			for (int SampleOffset = 0; SampleOffset < SamplesPerTexel; SampleOffset += SampleChunk)
			{
				ReflectionCaptureConvolveShader::Parameters ConvolveParams;
				ConvolveParams.SourceRadiance = Radiance;
				ConvolveParams.TargetMip = MipViews[Mip];
				ConvolveParams.Constants.Value = {
					.TargetMipSize = MipSize,
					.TargetMip = Mip,
					.MipCount = MipCount,
					.SampleOffset = SampleOffset,
					.SamplesThisPass = Math::Min(SampleChunk, SamplesPerTexel - SampleOffset),
					.TotalSamples = SamplesPerTexel,
				};

				RenderPassParameters PassParameters;
				RenderPassDependencies Dependencies(Graph.Allocator);
				// SourceRadiance registers the read; each chunk reads the previous accumulation
				Dependencies.Bind<ReflectionCaptureConvolveShader>(ConvolveParams);
				Dependencies.Write(Radiance, VK_ACCESS_SHADER_WRITE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

				Graph.AddPass("ReflectionCaptureConvolve", RenderGraphPassType::Compute, PassParameters, Dependencies, [ConvolveParams, MipSize](RenderGraphContext& Context)
				{
					RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterReflectionCaptureBake, Context);

					ComputePipeline* Pipeline = GetComputePipeline<ReflectionCaptureConvolveShader>(Context, ReflectionCaptureConvolveShader::Permutation{});
					Context.BindComputePipeline<ReflectionCaptureConvolveShader>(Pipeline, ConvolveParams);
					Context.CommandBuffer->Dispatch((u32)((MipSize + 7) / 8), (u32)((6 * MipSize + 7) / 8), 1);
				});
			}
		}

		// the graph borrows the views during execution; deferred destruction outlives it
		for (Texture2* MipView : MipViews)
		{
			Graph.Device->DestroyTextureDeferred(MipView);
		}
	}
}
