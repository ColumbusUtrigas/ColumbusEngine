#include "RenderGraph.h"
#include "Graphics/Texture.h"
#include "Graphics/Types.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"
#include "Graphics/Vulkan/ComputePipelineVulkan.h"
#include "Graphics/Vulkan/GraphicsPipelineVulkan.h"
#include "Graphics/Vulkan/RayTracingPipelineVulkan.h"
#include "Graphics/Vulkan/TextureVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "System/File.h"

#include <memory>
#include <vulkan/vulkan.hpp>

namespace Columbus
{

	Buffer* RenderGraphContext::GetOutputBuffer(const std::string& Name, const BufferDesc &Desc, void* InitialData)
	{
		if (RenderData.Buffers.find(Name) == RenderData.Buffers.end())
		{
			RenderData.Buffers[Name] = Device->CreateBuffer(Desc, InitialData);
		}

		return RenderData.Buffers[Name];
	}

	Buffer* RenderGraphContext::GetInputBuffer(const std::string& Name)
	{
		// TODO: sync
		return RenderData.Buffers[Name];
	}

	Texture2* RenderGraphContext::GetRenderTarget(const std::string& Name, const TextureDesc2& Desc)
	{
		if (RenderData.Textures.find(Name) == RenderData.Textures.end())
		{
			RenderData.Textures[Name] = Device->CreateTexture(Desc);
		}

		return RenderData.Textures[Name];
	}

	Texture2* RenderGraphContext::GetInputTexture(const std::string& Name)
	{
		TextureVulkan* Texture = static_cast<TextureVulkan*>(RenderData.Textures[Name]);

		VkImageMemoryBarrier Barrier{};
		Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		Barrier.oldLayout = Texture->_Layout; // TODO
		Barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL; // TODO
		Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		Barrier.image = Texture->_Image;
		Barrier.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(Texture->GetDesc().Format);
		Barrier.subresourceRange.baseMipLevel = 0;
		Barrier.subresourceRange.levelCount = 1;
		Barrier.subresourceRange.baseArrayLayer = 0;
		Barrier.subresourceRange.layerCount = 1;
		Barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // TODO
		Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT; // TODO

		vkCmdPipelineBarrier(CommandBuffer->_CmdBuf,
			VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // TODO
			0,
			0, nullptr,
			0, nullptr,
			1, &Barrier);

		Texture->_Layout = VK_IMAGE_LAYOUT_GENERAL; // TODO

		return Texture;
	}

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

	void RenderGraph::AddRenderPass(RenderPass* Pass)
	{
		RenderPasses.push_back(Pass);
	}

	void RenderGraph::Build()
	{
		for (auto Pass : RenderPasses)
		{
			if (Pass->IsGraphicsPass)
			{
				Pass->VulkanRenderPass = Device->CreateRenderPass(Pass->RenderTargets);

				// TODO
				for (auto& Target : Pass->RenderTargets)
				{
					if (Target.Name != RenderPass::FinalColorOutput && !RenderData.Textures.contains(Target.Name))
					{
						TextureDesc2 RTDesc;
						RTDesc.Width = 1280; // TODO
						RTDesc.Height = 720; // TODO
						RTDesc.Format = Target.Format;
						RTDesc.Usage = Target.Type == AttachmentType::Color ? TextureUsage::RenderTargetColor : TextureUsage::RenderTargetDepth;
						RenderData.Textures[Target.Name] = Device->CreateTexture(RTDesc);
					}
				}
				// Device->CreateFramebuffer();
			}
		}

		BlankPass = Device->CreateRenderPass(VK_FORMAT_B8G8R8A8_SRGB);
	}

	void RenderGraph::Execute(SwapchainVulkan* Swapchain)
	{
		static bool FirstTime = true;

		RenderData.CurrentPerFrameData = (RenderData.CurrentPerFrameData + 1) % MaxFramesInFlight;
		auto& PerFrameData = RenderData.PerFrameData[RenderData.CurrentPerFrameData];
		auto CommandBuffer = PerFrameData.CommandBuffer;

		Device->WaitForFence(PerFrameData.Fence, UINT64_MAX);
		Device->ResetFence(PerFrameData.Fence);

		uint32_t SwapchainImageIndex;
		Device->AcqureNextImage(Swapchain, PerFrameData.ImageSemaphore, SwapchainImageIndex);

		RenderGraphContext Context{BlankPass, Device, Scene, CommandBuffer, RenderData};
		CommandBuffer->Reset();
		CommandBuffer->Begin();

		if (FirstTime)
		{
			// Device->CreateFramebuffers(Swapchain, BlankPass); // TODO: better system

			for (auto Pass : RenderPasses)
			{
				if (Pass->IsGraphicsPass)
				{
					// TODO: refactor, make swapchain images integration more seamless
					for (int i = 0; i < Swapchain->imageCount; i++)
					{
						std::vector<Texture2*> TexturesForPass;
						for (const AttachmentDesc& TargetDesc : Pass->RenderTargets)
						{
							if (TargetDesc.Name == RenderPass::FinalColorOutput)
							{
								TexturesForPass.push_back(Swapchain->Textures[i]);
							}
							else
							{
								TexturesForPass.push_back(RenderData.Textures[TargetDesc.Name]);
							}
						}

						Pass->VulkanFramebuffers[i] = Device->CreateFramebuffer(Pass->VulkanRenderPass, TexturesForPass);
					}
				}

				Context.VulkanRenderPass = Pass->VulkanRenderPass;
				Pass->Setup(Context);
			}
			FirstTime = false;
		}

		for (auto Pass : RenderPasses)
		{
			Context.VulkanRenderPass = Pass->VulkanRenderPass;

			CommandBuffer->BeginDebugMarker(Pass->Name.c_str());
			Pass->PreExecute(Context);

			if (Pass->IsGraphicsPass)
			{
				fixed_vector<VkClearValue, 16> ClearValues;
				for (const AttachmentDesc& Attachment : Pass->RenderTargets)
				{
					VkClearValue ClearValue;
					ClearValue.color = {};
					ClearValue.depthStencil = {1,0};
					ClearValues.push_back(ClearValue); // TODO
				}

				CommandBuffer->BeginRenderPass(Pass->VulkanRenderPass, VkRect2D{{}, Swapchain->swapChainExtent}, Pass->VulkanFramebuffers[SwapchainImageIndex], ClearValues.size(), ClearValues.data());
				Pass->Execute(Context);
				CommandBuffer->EndRenderPass();

				// state tracking, TODO: refactor
				for (const AttachmentDesc& Attachment : Pass->RenderTargets)
				{
					if (Attachment.Name != RenderPass::FinalColorOutput) // TODO
					{
						static_cast<TextureVulkan*>(RenderData.Textures[Attachment.Name])->_Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO
					}
				}
			}
			else
			{
				Pass->Execute(Context);
			}

			CommandBuffer->EndDebugMarker();
		}

		CommandBuffer->End();

		Device->Submit(CommandBuffer, PerFrameData.Fence, 1, &PerFrameData.ImageSemaphore, 1, &PerFrameData.SubmitSemaphore);
		Device->Present(Swapchain, SwapchainImageIndex, PerFrameData.SubmitSemaphore);
	}

}
