#include "Core/Core.h"
#include "Core/CVar.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/ShaderCache.h"
#include "RenderPasses.h"

ConsoleVariable<int> CVar_FilmCurve("r.Tonemap.FilmCurve", "0 - ACES, 1 - AgX, 2 - Flim", 1);
ConsoleVariable<int> CVar_OutputTransform("r.Tonemap.OutputTransform", "0 - None, 1 - Rec.709, 2 - Rec.2020", 1);
ConsoleVariable<int> CVar_FilmGrain("r.FilmGrain", "Whether to allow using film grain", 1);

DECLARE_GPU_PROFILING_COUNTER(GpuCounterTonemap);

IMPLEMENT_GPU_PROFILING_COUNTER("Tonemap", "RenderGraphGPU", GpuCounterTonemap);

namespace Columbus
{

	// Image formation type
	enum class TonemapFilmCurve : u32
	{
		ACES,
		AgX,
		Flim
	};

	enum class TonemapOutputTransform : u32
	{
		None,
		Rec709,
		Rec2020
	};

	struct TonemapParameters
	{
		TonemapFilmCurve       FilmCurve;
		TonemapOutputTransform OutputTransform;

		iVector2 Resolution;

		float Vignette;
		float GrainScale;
		float GrainAmount;
		u32   GrainSeed;
	};

	struct TonemapTextures
	{
		RenderGraphTextureRef ColourGradingLUT;
		RenderGraphTextureRef TonemappedImage;
	};

	struct TonemapShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/Tonemap.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct PipelinePermutation
		{
			CullMode Cull = CullMode::No;
		};

		static GraphicsPipelineDesc BuildPipelineDesc(const PipelinePermutation& Permutation)
		{
			GraphicsPipelineDesc Desc;
			Desc.Name = "Tonemap";
			Desc.rasterizerState.Cull = Permutation.Cull;
			Desc.blendState.RenderTargets = { RenderTargetBlendDesc() };
			return Desc;
		}

		struct Parameters
		{
			ShaderSampledTexture SceneTexture;
			ShaderConstants<HColourCorrectionSettings> ColourCorrection;
			ShaderConstants<TonemapParameters> Constants;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.SceneTexture,     0, 0);
			Binder.Bind(Params.ColourCorrection, 0, 1);
			Binder.Bind(Params.Constants,        0, 2);
		}
	};

	struct ColourGradingLUTShader
	{
		static constexpr const char* Path = "./PrecompiledShaders/ComputeColourGradingLUT.csd";

		struct Permutation
		{
		};

		static void BuildPermutationLayout(ShaderPermutationLayoutBuilder<Permutation>& Builder)
		{
		}

		struct Parameters
		{
			ShaderStorageTexture ColourGradingLUT;
		};

		static void Bind(ShaderBinder& Binder, const Parameters& Params)
		{
			Binder.Bind(Params.ColourGradingLUT, 0, 0);
		}
	};

	void ComputeColourGradingLUT(RenderGraph& Graph, TonemapTextures& Textures)
	{
		// TODO: external LUTs
		
		static const iVector3 LutResolution { 32, 32, 32 };
		static const iVector3 GroupSize { 8, 8, 8 };

		{
			TextureDesc2 Desc {
				.Type   = TextureType::Texture3D,
				.Usage  = TextureUsage::Storage,
				.Width  = (u32)LutResolution.X, .Height = (u32)LutResolution.Y, .Depth = (u32)LutResolution.Z,
				.Format = TextureFormat::RGBA16F
			};
			Textures.ColourGradingLUT = Graph.CreateTexture(Desc, "ColourGradingLUT");
		}

		ColourGradingLUTShader::Parameters LUTParams;
		LUTParams.ColourGradingLUT = Textures.ColourGradingLUT;

		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Bind<ColourGradingLUTShader>(LUTParams);

		Graph.AddPass("ComputeColourGradingLUT", RenderGraphPassType::Compute, {}, Dependencies, [LUTParams](RenderGraphContext& Context)
		{
			ComputePipeline* Pipeline = GetComputePipeline<ColourGradingLUTShader>(Context, ColourGradingLUTShader::Permutation {});
			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.BindComputeParameters<ColourGradingLUTShader>(Pipeline, LUTParams);
			Context.DispatchComputePixels(Pipeline, GroupSize, LutResolution);
		});
	}

	RenderGraphTextureRef TonemapPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef SceneTexture)
	{
		TonemapTextures Textures;
		ComputeColourGradingLUT(Graph, Textures);

		TextureDesc2 Desc = Graph.GetTextureDesc(SceneTexture);

		iVector2 Size(Desc.Width, Desc.Height);

		TextureDesc2 TonemapTextureDesc {
			.Usage = TextureUsage::RenderTargetColor,
			.Width = (u32)Size.X, .Height = (u32)Size.Y,
			.Format = TextureFormat::BGRA8SRGB,
			.AddressU = TextureAddressMode::ClampToEdge,
			.AddressV = TextureAddressMode::ClampToEdge,
			.AddressW = TextureAddressMode::ClampToEdge,
		};
		Textures.TonemappedImage = Graph.CreateTexture(TonemapTextureDesc, "Tonemapped");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Textures.TonemappedImage };
		Parameters.ViewportSize = Size;

		RenderPassDependencies Dependencies(Graph.Allocator);
		TonemapShader::Permutation Permutation;
		TonemapShader::PipelinePermutation PipelinePermutation;

		const int FilmCurve = Math::Clamp(CVar_FilmCurve.GetValue(), 0, 2);
		const int OutputTransform = Math::Clamp(CVar_OutputTransform.GetValue(), 0, 1);
		const bool AllowFilmGrain = CVar_FilmGrain.GetValue() != 0 && View.EffectsSettings.FilmGrain.EnableGrain;

		TonemapShader::Parameters TonemapParams;
		TonemapParams.SceneTexture = SceneTexture;
		TonemapParams.ColourCorrection.Value = View.EffectsSettings.ColourCorrection;
		TonemapParams.Constants.Value = TonemapParameters {
			.FilmCurve       = (TonemapFilmCurve)FilmCurve,
			.OutputTransform = (TonemapOutputTransform)OutputTransform,
			.Resolution      = Size,
			.Vignette        = View.EffectsSettings.Vignette.Vignette * View.EffectsSettings.Vignette.EnableVignette,
			.GrainScale      = Math::Max(0.001f, View.EffectsSettings.FilmGrain.GrainScale),
			.GrainAmount     = View.EffectsSettings.FilmGrain.GrainAmount * AllowFilmGrain,
			.GrainSeed       = (u32)rand(),
		};

		Dependencies.Bind<TonemapShader>(TonemapParams);

		Graph.AddPass("Tonemap", RenderGraphPassType::Raster, Parameters, Dependencies, [TonemapParams, Permutation, PipelinePermutation](RenderGraphContext& Context)
		{
			RENDER_GRAPH_PROFILE_GPU_SCOPED(GpuCounterTonemap, Context);

			GraphicsPipeline* Pipeline = GetGraphicsPipeline<TonemapShader>(Context, Permutation, PipelinePermutation);
			Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
			Context.BindGraphicsParameters<TonemapShader>(Pipeline, TonemapParams);
			Context.CommandBuffer->Draw(3, 1, 0, 0);
		});

		return Textures.TonemappedImage;
	}

}
