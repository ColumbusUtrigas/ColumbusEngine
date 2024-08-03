#include "RenderPasses.h"

namespace Columbus::Antialiasing
{

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
		Graph.CreateHistoryTexture(&Textures.History.TAAHistory, Desc, "FinalHistory");

		Desc.Usage = TextureUsage::StorageSampled;

		Texture2* HistoryTexture = Textures.History.TAAHistory;
		RenderGraphTextureRef InputTexture = Textures.FinalBeforeTonemap;
		RenderGraphTextureRef VelocityTexture = Textures.Velocity;
		RenderGraphTextureRef OutputTexture = Textures.FinalBeforeTonemap;

		RenderPassParameters Parameters;
		RenderPassDependencies Dependencies(Graph.Allocator);
		Dependencies.Read(InputTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Read(VelocityTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		Dependencies.Write(OutputTexture, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

		iVector2 Size = View.RenderSize;

		Graph.AddPass("TAA", RenderGraphPassType::Compute, Parameters, Dependencies, [Size, InputTexture, VelocityTexture, OutputTexture, HistoryTexture](RenderGraphContext& Context)
		{
			static ComputePipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				ComputePipelineDesc Desc;
				Desc.Name = "TAA";
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/TAA.csd");
				Pipeline = Context.Device->CreateComputePipeline(Desc);
			}

			auto Set = Context.GetDescriptorSet(Pipeline, 0);
			Context.Device->UpdateDescriptorSet(Set, 0, 0, Context.GetRenderGraphTexture(InputTexture).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 1, 0, HistoryTexture, TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 2, 0, Context.GetRenderGraphTexture(VelocityTexture).get(), TextureBindingFlags::AspectColour, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			Context.Device->UpdateDescriptorSet(Set, 3, 0, Context.GetRenderGraphTexture(OutputTexture).get());
			Context.Device->UpdateDescriptorSet(Set, 4, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

			struct
			{
				iVector2 Resolution;
			} Params;
			Params.Resolution = Size;

			Context.CommandBuffer->BindComputePipeline(Pipeline);
			Context.CommandBuffer->BindDescriptorSetsCompute(Pipeline, 0, 1, &Set);
			Context.CommandBuffer->PushConstantsCompute(Pipeline, ShaderType::Compute, 0, sizeof(Params), &Params);
			Context.DispatchComputePixels(Pipeline, { 8,8,1 }, { Size, 1 });
		});

		Graph.ExtractTexture(OutputTexture, &Textures.History.TAAHistory);

		return OutputTexture;
	}

}