#include "RenderGraph.h"
#include "Common/Image/Image.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Core/Types.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include "Graphics/Vulkan/PipelinesVulkan.h"
#include "Graphics/Vulkan/TextureVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Core/Hash.h"
#include "Math/Vector2.h"

#include <algorithm>
#include <memory>
#include <vulkan/vulkan.hpp>

namespace std {
	template <> struct hash<Columbus::RenderPassAttachment>
	{
		size_t operator()(const Columbus::RenderPassAttachment & x) const
		{
			size_t Hash = 0;
			hash_combine(Hash, x.LoadOp);
			hash_combine(Hash, x.Texture);
			return Hash;
		}
	};
}

size_t Columbus::HashRenderPassParameters::operator()(const Columbus::RenderPassParameters& Params) const
{
	size_t Hash = 0;

	for (auto& A : Params.ColorAttachments)
		hash_combine(Hash, A);

	hash_combine(Hash, Params.DepthStencilAttachment);

	return Hash;
}

namespace Columbus
{

	// TODO: unify
	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const ComputePipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		auto& DescriptorSets = RenderData.DescriptorSets[RenderData.CurrentPerFrameData];
		if (DescriptorSets.find(vkpipe->pipeline) == DescriptorSets.end())
		{
			auto DescriptorSetData = RenderGraphData::PipelineDescriptorSetData();

			for (int i = 0; i < vkpipe->SetLayouts.UsedLayouts; i++)
			{
				DescriptorSetData.DescriptorSets[Index] = Device->CreateDescriptorSet(Pipeline, Index);
			}

			DescriptorSets[vkpipe->pipeline] = DescriptorSetData;
		}

		return DescriptorSets[vkpipe->pipeline].DescriptorSets[Index];
	}

	// TODO: unify
	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const GraphicsPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(Pipeline);

		auto& DescriptorSets = RenderData.DescriptorSets[RenderData.CurrentPerFrameData];
		if (DescriptorSets.find(vkpipe->pipeline) == DescriptorSets.end())
		{
			auto DescriptorSetData = RenderGraphData::PipelineDescriptorSetData();

			for (int i = 0; i < vkpipe->SetLayouts.UsedLayouts; i++)
			{
				DescriptorSetData.DescriptorSets[Index] = Device->CreateDescriptorSet(Pipeline, Index);
			}

			DescriptorSets[vkpipe->pipeline] = DescriptorSetData;
		}

		return DescriptorSets[vkpipe->pipeline].DescriptorSets[Index];
	}

	// TODO: unify
	VkDescriptorSet RenderGraphContext::GetDescriptorSet(const RayTracingPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(Pipeline);

		auto& DescriptorSets = RenderData.DescriptorSets[RenderData.CurrentPerFrameData];
		if (DescriptorSets.find(vkpipe->pipeline) == DescriptorSets.end())
		{
			auto DescriptorSetData = RenderGraphData::PipelineDescriptorSetData();

			for (int i = 0; i < vkpipe->SetLayouts.UsedLayouts; i++)
			{
				DescriptorSetData.DescriptorSets[i] = Device->CreateDescriptorSet(Pipeline, i);
			}

			DescriptorSets[vkpipe->pipeline] = DescriptorSetData;
		}

		return DescriptorSets[vkpipe->pipeline].DescriptorSets[Index];
	}

	void RenderGraphContext::BindGPUScene(const GraphicsPipeline* Pipeline)
	{
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &RenderData.GPUSceneData.VerticesSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 1, 1, &RenderData.GPUSceneData.IndicesSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 2, 1, &RenderData.GPUSceneData.UVSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 3, 1, &RenderData.GPUSceneData.NormalSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 4, 1, &RenderData.GPUSceneData.TextureSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 5, 1, &RenderData.GPUSceneData.MaterialSet);
		CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 6, 1, &RenderData.GPUSceneData.LightSet);
	}

	void RenderGraphContext::BindGPUScene(const RayTracingPipeline* Pipeline)
	{
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 0, 1, &RenderData.GPUSceneData.VerticesSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 1, 1, &RenderData.GPUSceneData.IndicesSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 2, 1, &RenderData.GPUSceneData.UVSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 3, 1, &RenderData.GPUSceneData.NormalSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 4, 1, &RenderData.GPUSceneData.TextureSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 5, 1, &RenderData.GPUSceneData.MaterialSet);
		CommandBuffer->BindDescriptorSetsRayTracing(Pipeline, 6, 1, &RenderData.GPUSceneData.LightSet);
	}

	SPtr<Texture2> RenderGraphContext::GetRenderGraphTexture(RenderGraphTextureRef Ref)
	{
		return Graph.Textures[Ref].Texture;
	}

	RenderGraph::RenderGraph(SPtr<DeviceVulkan> Device, SPtr<GPUScene> Scene) : Device(Device), Scene(Scene)
	{
		for (auto& PerFrameData : RenderData.PerFrameData)
		{
			PerFrameData.Fence = Device->CreateFence(true);
			PerFrameData.ImageSemaphore = Device->CreateSemaphore();
			PerFrameData.SubmitSemaphore = Device->CreateSemaphore();
			PerFrameData.CommandBuffer = Device->CreateCommandBuffer();
		}

		// Assuming that GPUScene is completely static, fill it in
		// TODO: fully refactor

		// brute-generate VkDescriptorSetLayout
		auto CreateLayout = [&](uint32_t bindingNum, VkDescriptorType type, uint32_t count, bool unbounded = true)
		{
			VkDescriptorBindingFlags bindingFlags = VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
			bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsInfo.pNext = nullptr;
			bindingFlagsInfo.bindingCount = 1;
			bindingFlagsInfo.pBindingFlags = &bindingFlags;

			VkDescriptorSetLayoutBinding binding;
			binding.binding = bindingNum;
			binding.descriptorType = type;
			binding.descriptorCount = count;
			binding.stageFlags = VK_SHADER_STAGE_ALL;
			binding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo setLayoutInfo;
			setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setLayoutInfo.pNext = unbounded ? &bindingFlagsInfo : nullptr;
			setLayoutInfo.flags = 0;
			setLayoutInfo.bindingCount = 1;
			setLayoutInfo.pBindings = &binding;

			VkDescriptorSetLayout layout;

			VK_CHECK(vkCreateDescriptorSetLayout(Device->_Device, &setLayoutInfo, nullptr, &layout));

			return layout;
		};

		RenderData.GPUSceneLayout.VerticesLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
		RenderData.GPUSceneLayout.IndicesLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
		RenderData.GPUSceneLayout.UVLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
		RenderData.GPUSceneLayout.NormalLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
		RenderData.GPUSceneLayout.TextureLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
		RenderData.GPUSceneLayout.MaterialLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
		RenderData.GPUSceneLayout.LightLayout = CreateLayout(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, false);

		RenderData.GPUSceneData.VerticesSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.IndicesLayout, 1000);
		RenderData.GPUSceneData.IndicesSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.IndicesLayout, 1000);
		RenderData.GPUSceneData.UVSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.UVLayout, 1000);
		RenderData.GPUSceneData.NormalSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.NormalLayout, 1000);
		RenderData.GPUSceneData.TextureSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.TextureLayout, 1000);
		RenderData.GPUSceneData.MaterialSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.MaterialLayout, 1000);
		RenderData.GPUSceneData.LightSet = Device->CreateDescriptorSetUnbounded(RenderData.GPUSceneLayout.LightLayout, 0);

		for (int i = 0; i < Scene->Meshes.size(); i++)
		{
			const auto& mesh = Scene->Meshes[i];
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.VerticesSet, 0, i, mesh.Vertices);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.IndicesSet, 0, i, mesh.Indices);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.UVSet, 0, i, mesh.UVs);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.NormalSet, 0, i, mesh.Normals);
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.MaterialSet, 0, i, mesh.Material);
		}

		for (int i = 0; i < Scene->Textures.size(); i++)
		{
			Device->UpdateDescriptorSet(RenderData.GPUSceneData.TextureSet, 0, i, Scene->Textures[i]);
		}

		auto Lights = GPUArray<GPULight>::Allocate(Scene->Lights.size());
		for (int i = 0; i < Scene->Lights.size(); i++)
		{
			(*Lights)[i] = Scene->Lights[i];
		}

		BufferDesc LightBufferDesc;
		LightBufferDesc.Size = Lights->Bytesize();
		LightBufferDesc.BindFlags = BufferType::UAV;
		auto LightBuffer = Device->CreateBuffer(LightBufferDesc, Lights.get());
		Device->SetDebugName(LightBuffer, "GPUScene.Lights");

		Device->UpdateDescriptorSet(RenderData.GPUSceneData.LightSet, 0, 0, LightBuffer);
	}

	RenderGraphTextureRef RenderGraph::CreateTexture(const TextureDesc2& Desc, const char* Name)
	{
		// on texture creation stage, we just create a description and a handle for it
		// the actual texture will be allocated on graph build stage

		int Id = static_cast<int>(Textures.size());
		Textures.push_back(RenderGraphTexture(Desc, Name, Id));
		return Id;
	}

	void RenderGraph::AllocateTexture(RenderGraphTexture& Texture)
	{
		const auto ApplyTextureFromPool = [&Texture, this](RenderGraphPooledTexture& PooledTexture) {
			Texture.Texture = PooledTexture.Texture;
			PooledTexture.Used = true;
			Device->SetDebugName(Texture.Texture.get(), Texture.DebugName.c_str());
		};

		if (TextureResourcePool.contains(Texture.Desc))
		{
			auto& TextureResources = TextureResourcePool[Texture.Desc];

			// Search for the first unused texture with matching Desc
			auto FoundRef = std::find_if(TextureResources.begin(), TextureResources.end(), [](RenderGraphPooledTexture& Tex) {
				return !Tex.Used;
			});

			if (FoundRef != TextureResources.end()) // Found
			{
				ApplyTextureFromPool(*FoundRef);
				return;
			}
		} else
		{
			// No matching Desc, ensure that list exists
			TextureResourcePool[Texture.Desc] = std::vector<RenderGraphPooledTexture>();
		}

		Log::Message("RenderGraph texture allocated: %s (%ix%ix%i)", Texture.DebugName.c_str(), Texture.Desc.Width, Texture.Desc.Height, Texture.Desc.Depth);

		// No available texture with matching Desc found, create a new one
		RenderGraphPooledTexture& Result = TextureResourcePool[Texture.Desc].emplace_back(
			RenderGraphPooledTexture{ SPtr<Texture2>(Device->CreateTexture(Texture.Desc)), false }
		);
		ApplyTextureFromPool(Result);
	}

	RenderGraphTextureRef RenderGraph::GetSwapchainTexture()
	{
		return SwapchainId;
	}

	void RenderGraph::AddPass(const std::string& Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback)
	{
		int PassId = static_cast<int>(Passes.size());
		RenderPass Pass{ Name, PassId, Type, Parameters, Dependencies, ExecuteCallback };

		// treat pass attachments as write dependencies
		for (const auto& Attachment : Parameters.ColorAttachments)
		{
			if (Attachment)
			{
				Pass.Dependencies.Write(Attachment->Texture, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
			}
		}

		if (Parameters.DepthStencilAttachment)
		{
			Pass.Dependencies.Write(Parameters.DepthStencilAttachment->Texture, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}

		Passes.emplace_back(std::move(Pass));
	}

	void RenderGraph::Clear()
	{
		Passes.clear();
		Textures.clear();

		// TODO: cleanup unused textures from the previous frame

		// TODO: inter-frame and global resource logic
		for (auto& DescPool : TextureResourcePool)
		{
			for (auto& PooledTexture : DescPool.second)
			{
				PooledTexture.Clear();
			}
		}
	}

	void RenderGraph::Build(Texture2* SwapchainImage)
	{
		// TODO: topological sort and graph reordering

		// TODO: virtual resources and allocation on build
		for (auto& Texture : Textures)
		{
			AllocateTexture(Texture);
		}

		const auto UpdateTextureLifetime = [](RenderGraphTexture& Texture, RenderGraphPassId PassId) {
			if (PassId < Texture.FirstUsage) Texture.FirstUsage = PassId;
			if (PassId > Texture.LastUsage)  Texture.LastUsage  = PassId;
		};

		// derive lifetime information about resources
		// assuming that Passes is topologically sorted
		for (auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				auto& GraphTexture = Textures[Read.Texture];

				// update resource lifetime
				UpdateTextureLifetime(GraphTexture, Pass.Id);

				// update resource consumers
				GraphTexture.Readers.push_back(GraphTexture.Id);
			}

			for (const auto& Write : Pass.Dependencies.TextureWriteResources)
			{
				auto& GraphTexture = Textures[Write.Texture];

				// update resource lifetime
				UpdateTextureLifetime(GraphTexture, Pass.Id);

				// update resource producer
				if (GraphTexture.Writer == -1)
				{
					GraphTexture.Writer = Pass.Id;
				}
				else
				{
					// validation error, resource can have only one producer
				}
			}
		}

		// knowing resource adjacent passes (writer and readers), setup barriers for a pass
		// needed only for reading passes
		for (auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				const auto& GraphTexture = Textures[Read.Texture];
				const auto& WriterPass = Passes[GraphTexture.Writer];

				// search for the writing dependency, TODO: better solution?
				for (const auto& WriterPassWrite : WriterPass.Dependencies.TextureWriteResources)
				{
					if (WriterPassWrite.Texture == Read.Texture)
					{
						Pass.TextureBarriers.push_back(RenderGraphTextureBarrier { GraphTexture.Id, WriterPassWrite, Read });
					}
				}
			}
		}

		// test: dump texture lifetime information, TODO: expose into debug dump
		#if 0
		for (auto& Texture : Textures)
		{
			printf("Texture lifetime: %s lives between %s and %s\n", Texture.DebugName.c_str(), Passes[Texture.FirstUsage].Name.c_str(), Passes[Texture.LastUsage].Name.c_str());
		}
		#endif

		// test: dump scheduling and barriers information, TODO: expose into debug dump
		#if 0
		{
			printf("RenderGraph Scheduling: ");
			for (auto& Pass : Passes)
			{
				printf("%s (barriers:", Pass.Name.c_str());
				for (auto& Barrier : Pass.TextureBarriers)
				{
					printf(" %s,", Textures[Barrier.Texture].DebugName.c_str());
				}
				printf("),");
			}
			printf("\n");
		}
		#endif

		// some vulkan-specific stuff for renderpasses
		for (auto& Pass : Passes)
		{
			if (Pass.Type == RenderGraphPassType::Raster)
			{
				Pass.VulkanRenderPass = GetOrCreateVulkanRenderPass(Pass);
				Pass.VulkanFramebuffer = GetOrCreateVulkanFramebuffer(Pass, SwapchainImage);
			}
		}
	}

	void RenderGraph::Execute(SwapchainVulkan* Swapchain)
	{
		RenderData.CurrentPerFrameData = (RenderData.CurrentPerFrameData + 1) % MaxFramesInFlight;
		auto& PerFrameData = RenderData.PerFrameData[RenderData.CurrentPerFrameData];
		auto CommandBuffer = PerFrameData.CommandBuffer;

		Device->WaitForFence(PerFrameData.Fence, UINT64_MAX);
		Device->ResetFence(PerFrameData.Fence);

		if (!Device->AcqureNextImage(Swapchain, PerFrameData.ImageSemaphore, RenderData.CurrentSwapchainImageIndex))
		{
			// swapchain invalidated
			// TODO: recreate swapchain
			Log::Fatal("Swapchain Invalidated");
		}

		{
			// Log::Message("Swapchain size: %ux%u", Swapchain->swapChainExtent.width, Swapchain->swapChainExtent.height);
			iVector2 SwapchainSize{ (int)Swapchain->swapChainExtent.width, (int)Swapchain->swapChainExtent.height };
			RenderData.CurrentSwapchainSize = SwapchainSize;
		}

		Build(Swapchain->Textures[RenderData.CurrentSwapchainImageIndex]);

		// TODO: multithreaded rendering
		RenderGraphContext Context{NULL, Device, Scene, CommandBuffer, RenderData, *this};
		CommandBuffer->Reset();
		CommandBuffer->Begin();

		for (auto& Pass : Passes)
		{
			// TODO: submarkers
			CommandBuffer->BeginDebugMarker(Pass.Name.c_str());

			// issue barriers for this pass
			for (const auto& Barrier : Pass.TextureBarriers)
			{
				const RenderGraphTexture& GraphTexture = Textures[Barrier.Texture];

				TextureVulkan* Texture = static_cast<TextureVulkan*>(GraphTexture.Texture.get());
				VkImageAspectFlags AspectFlags = TextureFormatToAspectMaskVk(Texture->GetDesc().Format);

				VkImageMemoryBarrier VkBarrier;
				VkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				VkBarrier.pNext = nullptr;
				VkBarrier.srcAccessMask = Barrier.Writer.Access;
				VkBarrier.dstAccessMask = Barrier.Reader.Access;
				VkBarrier.oldLayout = Texture->_Layout;
				VkBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				VkBarrier.srcQueueFamilyIndex = 0;
				VkBarrier.dstQueueFamilyIndex = 0;
				VkBarrier.image = Texture->_Image;
				VkBarrier.subresourceRange.aspectMask = AspectFlags;
				VkBarrier.subresourceRange.baseMipLevel = 0;
				VkBarrier.subresourceRange.levelCount = 1;
				VkBarrier.subresourceRange.baseArrayLayer = 0;
				VkBarrier.subresourceRange.layerCount = 1;

				vkCmdPipelineBarrier(CommandBuffer->_CmdBuf, Barrier.Writer.Stage, Barrier.Reader.Stage,
					0, 0, // memory barrier
					0, 0, // buffer memory barrier
					0, 1, (VkImageMemoryBarrier*)&VkBarrier);

				Texture->_Layout = VK_IMAGE_LAYOUT_GENERAL; // TODO: proper tracking system
			}

			if (Pass.Type == RenderGraphPassType::Raster)
			{
				fixed_vector<VkClearValue, 16> ClearValues;
				for (const auto& Attachment : Pass.Parameters.ColorAttachments)
				{
					if (Attachment && Attachment->LoadOp == AttachmentLoadOp::Clear)
					{
						ClearValues.push_back(AttachmentClearValueToVk(Attachment->ClearValue, false));
					}
				}

				if (Pass.Parameters.DepthStencilAttachment && Pass.Parameters.DepthStencilAttachment->LoadOp == AttachmentLoadOp::Clear)
				{
					ClearValues.push_back(AttachmentClearValueToVk(Pass.Parameters.DepthStencilAttachment->ClearValue, true));
				}

				Context.VulkanRenderPass = Pass.VulkanRenderPass;

				// TODO: resize, extent
				CommandBuffer->BeginRenderPass(Pass.VulkanRenderPass, VkRect2D{{}, Swapchain->swapChainExtent}, Pass.VulkanFramebuffer, ClearValues.size(), ClearValues.data());
				Pass.ExecutionFunc(Context);
				CommandBuffer->EndRenderPass();

				{
					// raster pass transitions attachments layouts
					// TODO: better system?
					for (auto& Attachment : Pass.Parameters.ColorAttachments)
					{
						if (Attachment && Textures[Attachment->Texture].Texture)
						{
							TextureVulkan* Texture = static_cast<TextureVulkan*>(Textures[Attachment->Texture].Texture.get());
							if (Texture != nullptr)
							{
								Texture->_Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
							}
						}
					}

					if (Pass.Parameters.DepthStencilAttachment)
					{
						TextureVulkan* Texture = static_cast<TextureVulkan*>(Textures[Pass.Parameters.DepthStencilAttachment->Texture].Texture.get());
						if (Texture != nullptr)
						{
							Texture->_Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						}
					}
				}
			}
			else
			{
				Pass.ExecutionFunc(Context);
			}

			CommandBuffer->EndDebugMarker();
		}

		// swapchain barrier
		{
			TextureVulkan* Texture = static_cast<TextureVulkan*>(Swapchain->Textures[RenderData.CurrentSwapchainImageIndex]);

			vk::ImageAspectFlags AspectFlags = (vk::ImageAspectFlags)TextureFormatToAspectMaskVk(TextureFormat::RGBA8);
			vk::ImageMemoryBarrier Barrier(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eNone,
				vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, 0, 0, Texture->_Image,
				vk::ImageSubresourceRange(AspectFlags, 0, 1, 0, 1));

			vkCmdPipelineBarrier(CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, // memory barrier
				0, 0, // buffer memory barrier
				0, 1, (VkImageMemoryBarrier*)&Barrier);
		}

		CommandBuffer->End();

		Device->Submit(CommandBuffer, PerFrameData.Fence, 1, &PerFrameData.ImageSemaphore, 1, &PerFrameData.SubmitSemaphore);
		Device->Present(Swapchain, RenderData.CurrentSwapchainImageIndex, PerFrameData.SubmitSemaphore);
	}

	std::string RenderGraph::ExportGraphviz()
	{
		// TODO: colour-mark nodes

		std::string Result;

		Result += "digraph G {\n";

		for (const auto& Pass : Passes)
		{
			Result += Pass.Name + " [shape=circle]\n";
		}

		for (const auto& Texture : Textures)
		{
			Result += Texture.DebugName + " [shape=box]\n";
		}

		for (const auto& Pass : Passes)
		{
			for (const auto& Read : Pass.Dependencies.TextureReadResources)
			{
				Result += Textures[Read.Texture].DebugName + " -> " + Pass.Name + "\n";
			}

			for (const auto& Write : Pass.Dependencies.TextureWriteResources)
			{
				if (Write.Texture)
				{
					Result += Pass.Name + " -> " + Textures[Write.Texture].DebugName + "\n";
				}
			}
		}

		Result += "}\n";

		return Result;
	}

	void RenderGraph::PrintDebugInformation()
	{

	}

	VkRenderPass RenderGraph::GetOrCreateVulkanRenderPass(RenderPass& Pass)
	{
		if (!MapOfVulkanRenderPasses.contains(Pass.Parameters))
		{
			Log::Message("Call: CreateRenderPass");

			// TODO: refactor + validation
			std::vector<AttachmentDesc> Attachments;
			for (auto& ColorAttachment : Pass.Parameters.ColorAttachments)
			{
				if (ColorAttachment)
				{
					// TODO: refactor + validation
					TextureFormat Format = ColorAttachment->Texture != SwapchainId ? Textures[ColorAttachment->Texture].Texture->GetDesc().Format : TextureFormat::BGRA8SRGB;
					Attachments.push_back(AttachmentDesc("", AttachmentType::Color, ColorAttachment->LoadOp, Format));
				}
			}

			if (Pass.Parameters.DepthStencilAttachment)
			{
				// TODO: refactor + validation
				auto Attachment = Pass.Parameters.DepthStencilAttachment;
				Attachments.push_back(AttachmentDesc("", AttachmentType::DepthStencil, Attachment->LoadOp, Textures[Attachment->Texture].Texture->GetDesc().Format));
			}

			MapOfVulkanRenderPasses[Pass.Parameters] = Device->CreateRenderPass(Attachments);
		}

		return MapOfVulkanRenderPasses[Pass.Parameters];
	}

	VkFramebuffer RenderGraph::GetOrCreateVulkanFramebuffer(RenderPass& Pass, Texture2* SwapchainImage)
	{
		int MapIndex = RenderData.CurrentSwapchainImageIndex;

		bool IsCached = MapOfVulkanFramebuffers[MapIndex].contains(Pass.VulkanRenderPass);
		bool InvalidateFramebuffer = false;

		if (IsCached && MapOfVulkanFramebuffers[MapIndex][Pass.VulkanRenderPass].Size != RenderData.CurrentSwapchainSize)
		{
			InvalidateFramebuffer = true;
		}

		if (!IsCached || InvalidateFramebuffer)
		{
			iVector2 Size = RenderData.CurrentSwapchainSize;
			Log::Message("Call: CreateFramebuffer: %ix%i", Size.X, Size.Y);

			// TODO: refactor + validation
			std::vector<Texture2*> PassTextures;
			for (auto& Color : Pass.Parameters.ColorAttachments)
			{
				if (Color)
				{
					// TODO: refactor + validation
					PassTextures.push_back(Color->Texture != SwapchainId ? Textures[Color->Texture].Texture.get() : SwapchainImage);
				}
			}

			if (Pass.Parameters.DepthStencilAttachment)
			{
				// TODO: refactor + validation
				PassTextures.push_back(Textures[Pass.Parameters.DepthStencilAttachment->Texture].Texture.get());
			}

			// TODO: use size of renderpass
			RenderGraphFramebufferVulkan FB;
			FB.VulkanFramebuffer = Device->CreateFramebuffer(Pass.VulkanRenderPass, Size, PassTextures);
			FB.Size = Size;

			MapOfVulkanFramebuffers[MapIndex][Pass.VulkanRenderPass] = FB;
		}

		return MapOfVulkanFramebuffers[MapIndex][Pass.VulkanRenderPass].VulkanFramebuffer;
	}

}