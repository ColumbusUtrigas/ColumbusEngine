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

	// TODO: LEGACY
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

	RenderGraphTextureRef RenderGraph::CreateTexture(const TextureDesc2& Desc, const char* Name)
	{
		if (TextureResourceTable.contains(Desc))
		{
			// Search for the first unused texture with matching Desc
			auto& TextureResources = TextureResourceTable[Desc];
			auto FoundRef = std::find_if(TextureResources.begin(), TextureResources.end(), [](RenderGraphTextureRef& Ref) {
				return Ref.use_count() == 1;
			});

			if (FoundRef != TextureResources.end())
			{
				// Found
				Device->SetDebugName(FoundRef->get(), Name);
				return *FoundRef;
			}
		}
		else
		{
			// No matching Desc, ensure that list exists
			TextureResourceTable[Desc] = std::vector<RenderGraphTextureRef>();
		}

		Log::Message("RenderGraph texture created: %s", Name);

		// No available texture with matching Desc found, create a new one
		RenderGraphTextureRef Result = TextureResourceTable[Desc].emplace_back(RenderGraphTextureRef(Device->CreateTexture(Desc)));
		Device->SetDebugName(Result.get(), Name);

		return Result;
	}

	Texture2* RenderGraph::GetSwapchainTexture()
	{
		return nullptr;
	}

	void RenderGraph::AddRenderPass(RenderPass* Pass)
	{
		RenderPasses.push_back(Pass);
	}

	void RenderGraph::AddPass(const std::string& Name, RenderGraphPassType Type, RenderPassParameters Parameters, RenderPassDependencies Dependencies, RenderGraphExecutionFunc ExecuteCallback)
	{
		RenderPass2 Pass{ Name, Type, Parameters, Dependencies, ExecuteCallback };
		Passes.emplace_back(std::move(Pass));
	}

	void RenderGraph::Clear()
	{
		Passes.clear();
	}

	void RenderGraph::Build(Texture2* SwapchainImage)
	{
		for (auto& Pass : Passes)
		{
			if (Pass.Type == RenderGraphPassType::Raster)
			{
				Pass.VulkanRenderPass = GetOrCreateVulkanRenderPass(Pass);
				Pass.VulkanFramebuffer = GetOrCreateVulkanFramebuffer(Pass, SwapchainImage);
				// TODO: recycle
				// TODO: setup barriers
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
			iVector2 SwapchainSize{ (int)Swapchain->swapChainExtent.width, (int)Swapchain->swapChainExtent.height };
			RenderData.CurrentSwapchainSize = SwapchainSize;
		}

		Build(Swapchain->Textures[RenderData.CurrentSwapchainImageIndex]);

		// TODO: multithreaded rendering
		RenderGraphContext Context{NULL, Device, Scene, CommandBuffer, RenderData};
		CommandBuffer->Reset();
		CommandBuffer->Begin();

		for (auto& Pass : Passes)
		{
			// TODO: submarkers
			CommandBuffer->BeginDebugMarker(Pass.Name.c_str());

			// Where to store producer flags??
			// Collect write-resources - to know resource producer flags
			// Collect read-resources
			// Validate resource usage
			// Place barrier

			// TODO: barriers
			for (RenderPassTextureDependency& Dependency : Pass.Dependencies.TextureReadResources)
			{
				// TODO: multithread/task concerns??
				// TODO: check if resource needs sync
				// TODO: refactor
				// place barrier
				TextureVulkan* Texture = static_cast<TextureVulkan*>(Dependency.Texture.get());

				vk::ImageAspectFlags AspectFlags = (vk::ImageAspectFlags)TextureFormatToAspectMaskVk(Texture->GetDesc().Format);
				vk::ImageMemoryBarrier Barrier(vk::AccessFlagBits::eShaderWrite, (vk::AccessFlags)Dependency.Access,
					vk::ImageLayout(Texture->_Layout), vk::ImageLayout::eGeneral, 0, 0, Texture->_Image,
					vk::ImageSubresourceRange(AspectFlags, 0, 1, 0, 1));

				// TODO: how to track write/read flags???
				vkCmdPipelineBarrier(CommandBuffer->_CmdBuf, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, Dependency.Stage,
					0, 0, // memory barrier
					0, 0, // buffer memory barrier
					0, 1, (VkImageMemoryBarrier*)&Barrier);

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
			}
			else
			{
				Pass.ExecutionFunc(Context);
			}

			CommandBuffer->EndDebugMarker();
		}

		CommandBuffer->End();

		Device->Submit(CommandBuffer, PerFrameData.Fence, 1, &PerFrameData.ImageSemaphore, 1, &PerFrameData.SubmitSemaphore);
		Device->Present(Swapchain, RenderData.CurrentSwapchainImageIndex, PerFrameData.SubmitSemaphore);
	}

	VkRenderPass RenderGraph::GetOrCreateVulkanRenderPass(RenderPass2& Pass)
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
					TextureFormat Format = ColorAttachment->Texture ? ColorAttachment->Texture->GetDesc().Format : TextureFormat::BGRA8SRGB;
					Attachments.push_back(AttachmentDesc("", AttachmentType::Color, ColorAttachment->LoadOp, Format));
				}
			}

			if (Pass.Parameters.DepthStencilAttachment)
			{
				// TODO: refactor + validation
				auto Attachment = Pass.Parameters.DepthStencilAttachment;
				Attachments.push_back(AttachmentDesc("", AttachmentType::DepthStencil, Attachment->LoadOp, Attachment->Texture->GetDesc().Format));
			}

			MapOfVulkanRenderPasses[Pass.Parameters] = Device->CreateRenderPass(Attachments);
		}

		return MapOfVulkanRenderPasses[Pass.Parameters];
	}

	VkFramebuffer RenderGraph::GetOrCreateVulkanFramebuffer(RenderPass2& Pass, Texture2* SwapchainImage)
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
			std::vector<Texture2*> Textures;
			for (auto& Color : Pass.Parameters.ColorAttachments)
			{
				if (Color)
				{
					// TODO: refactor + validation
					Textures.push_back(Color->Texture != nullptr ? Color->Texture : SwapchainImage);
				}
			}

			if (Pass.Parameters.DepthStencilAttachment)
			{
				// TODO: refactor + validation
				Textures.push_back(Pass.Parameters.DepthStencilAttachment->Texture);
			}

			// TODO: use size of renderpass
			RenderGraphFramebufferVulkan FB;
			FB.VulkanFramebuffer = Device->CreateFramebuffer(Pass.VulkanRenderPass, Size,Textures);
			FB.Size = Size;

			MapOfVulkanFramebuffers[MapIndex][Pass.VulkanRenderPass] = FB;
		}

		return MapOfVulkanFramebuffers[MapIndex][Pass.VulkanRenderPass].VulkanFramebuffer;
	}

}
