#include "CommandBufferVulkan.h"

#include "Common/Image/Image.h"
#include "Core/Assert.h"
#include "Core/CVar.h"
#include "Core/fixed_vector.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/Common.h"
#include "Profiling/Profiling.h"
#include "TypeConversions.h"
#include "DeviceVulkan.h"

#include "Counters.h"
#include "PipelinesVulkan.h"
#include "BufferVulkan.h"
#include "TextureVulkan.h"
#include "QueryPoolVulkan.h"

#include <cstring>
#include <memory>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>

IMPLEMENT_MEMORY_PROFILING_COUNTER("Buffers", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, MemoryCounter_Vulkan_AllocatedBuffers);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Host-visible buffers", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, MemoryCounter_Vulkan_AllocatedHostVisibleBuffers);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Images", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, MemoryCounter_Vulkan_AllocatedImages);

IMPLEMENT_CPU_PROFILING_COUNTER("Submit Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_SubmitTime, true);
IMPLEMENT_CPU_PROFILING_COUNTER("QueueWaitIdle Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_QueueWaitIdleTime, true);
IMPLEMENT_CPU_PROFILING_COUNTER("Fence Wait Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_FenceWaitTime, true);
IMPLEMENT_CPU_PROFILING_COUNTER("AcquireImage Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_AcquireImageTime, true);
IMPLEMENT_CPU_PROFILING_COUNTER("Present Time", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CpuCounter_Vulkan_PresentTime, true);

IMPLEMENT_COUNTING_PROFILING_COUNTER("Buffers count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_Buffers, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("CBuffers count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_CBuffers, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Images count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_Images, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Pipeline layouts count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_PipelineLayouts, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Descriptor sets count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_DescriptorSets, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Render passes count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_RenderPasses, false);
IMPLEMENT_COUNTING_PROFILING_COUNTER("Framebuffers count", PROFILING_CATEGORY_VULKAN_LOW_LEVEL, CountingCounter_Vulkan_Framebuffers, false);

ConsoleVariable<bool> RayTracing_CVar("r.RayTracing", "Controls whether ray tracing features are supported", true);

namespace Columbus
{

	void CBufferPoolVulkan::BeginFrame()
	{
		CurrentFrame = (CurrentFrame + 1) % MaxFramesInFlight;
		for (auto& entry : Pool[CurrentFrame])
			entry.Used = false;
	}

	DeviceVulkan::DeviceVulkan(VkPhysicalDevice PhysicalDevice, VkInstance Instance) :
		_PhysicalDevice(PhysicalDevice), _Instance(Instance)
	{
		VkFunctions.LoadFunctions(Instance);

		_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		_AccelerationStructureFeatures.pNext = nullptr;
		_RayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		_RayTracingFeatures.pNext = &_AccelerationStructureFeatures;
		//_RayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
		//_RayQueryFeatures.pNext = &_RayTracingFeatures;

		_Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		_Vulkan12Features.pNext = ENABLE_RAY_TRACING ? &_RayTracingFeatures : nullptr;
		_DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		_DeviceFeatures.pNext = &_Vulkan12Features;

		vkGetPhysicalDeviceFeatures2(PhysicalDevice, &_DeviceFeatures);

		_RayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
		_RayTracingProperties.pNext = nullptr;
		_DeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		_DeviceProperties.pNext = ENABLE_RAY_TRACING ? &_RayTracingProperties : nullptr;

		vkGetPhysicalDeviceProperties2(PhysicalDevice, &_DeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &_MemoryProperties);

		// logic if features are enabled/disabled
		COLUMBUS_ASSERT(_Vulkan12Features.descriptorIndexing); // non-uniform descriptor indexing
		COLUMBUS_ASSERT(_Vulkan12Features.descriptorBindingPartiallyBound);
		COLUMBUS_ASSERT(_Vulkan12Features.bufferDeviceAddress);
#if ENABLE_RAY_TRACING
		// TOOD: make some features optional
		COLUMBUS_ASSERT(_RayTracingFeatures.rayTracingPipeline);
#endif

		Log::Message("Creating Vulkan logical device on %s", _DeviceProperties.properties.deviceName);

		// enumerate queue families
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> qFamProps(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(_PhysicalDevice, &queueFamilyCount, qFamProps.data());

		// find first suitable queue family
		_FamilyIndex = -1;
		for (uint32 i = 0; i < qFamProps.size(); i++)
		{
			if (qFamProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				_FamilyIndex = i;
				break;
			}
		}

		COLUMBUS_ASSERT_MESSAGE(_FamilyIndex != -1, "Failed to choose queue family");

		float queuePriorities = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = _FamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriorities;

		std::vector<const char*> extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,

#if ENABLE_RAY_TRACING
				VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
#endif
		};

		if (IsVulkanDebugEnabled())
		{
			extensions.push_back(VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME);
		}

		VkDeviceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.pNext = &_DeviceFeatures;
		info.flags = 0;
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queueCreateInfo;
		info.enabledLayerCount = 0;
		info.ppEnabledLayerNames = nullptr;
		info.enabledExtensionCount = extensions.size();
		info.ppEnabledExtensionNames = extensions.data();
		info.pEnabledFeatures = nullptr;

		VK_CHECK(vkCreateDevice(PhysicalDevice, &info, nullptr, &_Device));

		_ComputeQueue = SmartPointer<VkQueue>(new VkQueue);
		vkGetDeviceQueue(_Device, _FamilyIndex, 0, _ComputeQueue.Get());

		// create buffer pool
		VkCommandPoolCreateInfo commandPoolInfo;
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.pNext = nullptr;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = _FamilyIndex;

		VK_CHECK(vkCreateCommandPool(_Device, &commandPoolInfo, nullptr, &_CmdPool));

		fixed_vector<VkDescriptorPoolSize, 16> poolSizes;
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4000 });
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000 });
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER, 4000 });
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 });
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 });
#if ENABLE_RAY_TRACING
		poolSizes.push_back(VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 5 });
#endif

		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.flags = 0;
		descriptorPoolInfo.maxSets = 1024;
		descriptorPoolInfo.poolSizeCount = poolSizes.size();
		descriptorPoolInfo.pPoolSizes = poolSizes.data();

		VK_CHECK(vkCreateDescriptorPool(_Device, &descriptorPoolInfo, nullptr, &_DescriptorPool));

		// initialise VMA
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.physicalDevice = PhysicalDevice;
		allocatorInfo.device = _Device;
		allocatorInfo.instance = Instance;
		allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &_Allocator));

		// initialise profiling
		_Profiler.Device = this;
		_Profiler.Init();

		// initialise upload ring
		UploadRing.Device = this;
		UploadRing.Init();

		// initialise default textures
		{
			Image img;
			img.Width = 1;
			img.Height = 1;
			img.Format = TextureFormat::RGBA8;

			u32 White = 0xFFFFFFFF;
			u32 Black = 0x00000000;
			u32 Transparent = 0xFFFFFF00;

			img.Data = (u8*)&White;
			DefaultTextures.White = CreateTexture(img);

			img.Data = (u8*)&Black;
			DefaultTextures.Black = CreateTexture(img);

			img.Data = (u8*)&Transparent;
			DefaultTextures.Transparent = CreateTexture(img);

			img.Data = nullptr;
		}
	}

	DeviceVulkan::~DeviceVulkan()
	{
		VK_CHECK(vkDeviceWaitIdle(_Device));

		DestroyTexture(DefaultTextures.White);
		DestroyTexture(DefaultTextures.Black);
		DestroyTexture(DefaultTextures.Transparent);

		_Profiler.Shutdown();
		UploadRing.Shutdown();

		//vmaDestroyAllocator(_Allocator);
		vkDestroyDescriptorPool(_Device, _DescriptorPool, nullptr);
		vkDestroyCommandPool(_Device, _CmdPool, nullptr);
		vkDestroyDevice(_Device, nullptr);
	}

	bool DeviceVulkan::SupportsRayTracing() const
	{
		return ENABLE_RAY_TRACING && RayTracing_CVar.GetValue() && _RayTracingFeatures.rayTracingPipeline;
	}

	bool DeviceVulkan::SupportsRayQuery() const
	{
		return ENABLE_RAY_TRACING && RayTracing_CVar.GetValue() && _RayQueryFeatures.rayQuery;
	}

	SwapchainVulkan* DeviceVulkan::CreateSwapchain(VkSurfaceKHR surface, SwapchainVulkan* OldSwapchain)
	{
		return new SwapchainVulkan(_Device, _PhysicalDevice, surface, OldSwapchain);
	}

	VkRenderPass DeviceVulkan::CreateRenderPass(std::span<AttachmentDesc> Attachments)
	{
		fixed_vector<VkAttachmentDescription, 16> VkAttachmentDescs;
		fixed_vector<VkAttachmentReference, 16> VkColorAttachmentRefs;
		VkAttachmentReference VkDepthStencilAttachmentRef;
		VkAttachmentReference* pVkDepthStencilAttachmentRef = nullptr;

		VkPipelineStageFlags SrcStageMask = 0;
		VkPipelineStageFlags DstStageMask = 0;
		VkAccessFlags DstAccessMask = 0;

		for (size_t i = 0; i < Attachments.size(); i++)
		{
			VkAttachmentDescription Attachment{};
			Attachment.format = TextureFormatToVK(Attachments[i].Format);
			Attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			// Attachment.storeOp = AttachmentStoreOpToVK(Attachments[i].StoreOp);
			Attachment.loadOp = AttachmentLoadOpToVk(Attachments[i].LoadOp);
			Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // TODO?
			Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; /// TODO
			Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // TODO

			if (Attachments[i].LoadOp == AttachmentLoadOp::Load)
			{
				// Attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO
				Attachment.initialLayout = VK_IMAGE_LAYOUT_GENERAL; // TODO
			} else
			{
				Attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO
			}
			Attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // TODO

			VkAttachmentReference Reference{};
			Reference.attachment = i;

			switch (Attachments[i].Type)
			{
			case AttachmentType::Color:
				SrcStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				DstStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				DstAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				Reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				VkColorAttachmentRefs.push_back(Reference);
				break;
			case AttachmentType::DepthStencil:
				SrcStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				DstStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
				DstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				Reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				VkDepthStencilAttachmentRef = Reference;
				pVkDepthStencilAttachmentRef = &VkDepthStencilAttachmentRef;
				break;
			}

			VkAttachmentDescs.push_back(Attachment);
		}

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = VkColorAttachmentRefs.size();
		subpass.pColorAttachments = VkColorAttachmentRefs.data();
		subpass.pDepthStencilAttachment = pVkDepthStencilAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = SrcStageMask;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = DstStageMask;
		dependency.dstAccessMask = DstAccessMask;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = VkAttachmentDescs.size();
		renderPassInfo.pAttachments = VkAttachmentDescs.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkRenderPass renderPass;

		VK_CHECK(vkCreateRenderPass(_Device, &renderPassInfo, nullptr, &renderPass));

		AddProfilingCount(CountingCounter_Vulkan_RenderPasses, 1);

		return renderPass;
	}

	VkFramebuffer DeviceVulkan::CreateFramebuffer(VkRenderPass Renderpass, const iVector2& Size, std::span<Texture2*> Textures)
	{
		fixed_vector<VkImageView, 16> AttachmentViews;
		for (Texture2* Attachment : Textures)
		{
			AttachmentViews.push_back(static_cast<TextureVulkan*>(Attachment)->_View);
		}

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = Renderpass;
		framebufferInfo.attachmentCount = AttachmentViews.size();
		framebufferInfo.pAttachments = AttachmentViews.data();
		framebufferInfo.width = Size.X;
		framebufferInfo.height = Size.Y;
		framebufferInfo.layers = 1;

		VkFramebuffer Result;

		VK_CHECK(vkCreateFramebuffer(_Device, &framebufferInfo, nullptr, &Result));

		AddProfilingCount(CountingCounter_Vulkan_Framebuffers, 1);

		return Result;
	}

	void DeviceVulkan::DestroyRenderPassDeferred(VkRenderPass Renderpass)
	{
		RenderPassDeferredDestroys.push_back(ResourceDeferredDestroyVulkan<VkRenderPass> {
			.Resource = Renderpass,
			.FramesLasted = 0,
		});
	}

	void DeviceVulkan::DestroyFramebufferDeferred(VkFramebuffer Framebuffer)
	{
		FramebufferDeferredDestroys.push_back(ResourceDeferredDestroyVulkan<VkFramebuffer> {
			.Resource = Framebuffer,
			.FramesLasted = 0,
		});
	}

	CommandBufferVulkan* DeviceVulkan::CreateCommandBuffer()
	{
		VkCommandBufferAllocateInfo commandBufferInfo;
		commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferInfo.pNext = nullptr;
		commandBufferInfo.commandPool = _CmdPool;
		commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferInfo.commandBufferCount = 1;

		VkCommandBuffer result;
		VK_CHECK(vkAllocateCommandBuffers(_Device, &commandBufferInfo, &result));

		return new CommandBufferVulkan(_Device, _CmdPool, result, VkFunctions);
	}

	SPtr<CommandBufferVulkan> DeviceVulkan::CreateCommandBufferShared()
	{
		return std::shared_ptr<CommandBufferVulkan>(CreateCommandBuffer());
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSetUnbounded(VkDescriptorSetLayout Layout, int MaxCount)
	{
		PipelineDescriptorSetLayoutsVulkan Layouts;
		Layouts.Layouts[0] = Layout;
		Layouts.VariableCountMax[0] = MaxCount;
		Layouts.UsedLayouts = 1;

		return _CreateDescriptorSet(Layouts, 0);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const ComputePipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const ComputePipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const GraphicsPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const GraphicsPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
	}

	VkDescriptorSet DeviceVulkan::CreateDescriptorSet(const RayTracingPipeline* Pipeline, int Index)
	{
		auto vkpipe = static_cast<const RayTracingPipelineVulkan*>(Pipeline);

		return _CreateDescriptorSet(vkpipe->SetLayouts, Index);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Buffer* Buffer)
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = static_cast<const BufferVulkan*>(Buffer)->_Buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkDescriptorType Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		if (Buffer->GetDesc().BindFlags == BufferType::Constant)
		{
			Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}

		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;
		write.descriptorType = Type;
		write.pImageInfo = nullptr;
		write.pBufferInfo = &bufferInfo;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Texture2* Texture, TextureBindingFlags Flags, VkDescriptorType DescriptorType)
	{
		auto vktex = static_cast<const TextureVulkan*>(Texture);

		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = vktex->_Layout;
		imageInfo.imageView = vktex->_View;
		imageInfo.sampler = vktex->_Sampler;

		if ((Flags & TextureBindingFlags::AspectColour) != 0)
		{
			imageInfo.imageView = vktex->_View;
		}
		else if ((Flags & TextureBindingFlags::AspectDepth) != 0 && (Flags & TextureBindingFlags::AspectStencil) != 0)
		{
			imageInfo.imageView = vktex->_View;
		}
		else if ((Flags & TextureBindingFlags::AspectDepth) != 0)
		{
			imageInfo.imageView = vktex->_DepthView;
		}
		else if ((Flags & TextureBindingFlags::AspectStencil) != 0)
		{
			imageInfo.imageView = vktex->_StencilView;
		} else
		{
			COLUMBUS_ASSERT_MESSAGE(false, "Wrong aspects for texture binding");
		}

		VkWriteDescriptorSet write;
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;

		// TODO: refactor binding system
		if (DescriptorType == VK_DESCRIPTOR_TYPE_MAX_ENUM)
			write.descriptorType = TextureUsageToVkDescriptorType(Texture->GetDesc().Usage);
		else
			write.descriptorType = DescriptorType;

		write.pImageInfo = &imageInfo;
		write.pBufferInfo = nullptr;
		write.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const AccelerationStructure* TLAS)
	{
		VkWriteDescriptorSetAccelerationStructureKHR ASinfo{};
		ASinfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		ASinfo.accelerationStructureCount = 1;
		ASinfo.pAccelerationStructures = &static_cast<const AccelerationStructureVulkan*>(TLAS)->_Handle;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		// The specialized acceleration structure descriptor has to be chained
		write.pNext = &ASinfo;
		write.dstSet = Set;
		write.dstBinding = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	void DeviceVulkan::UpdateDescriptorSet(VkDescriptorSet Set, int BindingId, int ArrayId, const Sampler* Sam)
	{
		VkDescriptorImageInfo imageInfo{};
		imageInfo.sampler = static_cast<const SamplerVulkan*>(Sam)->_Sampler;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.pNext = nullptr;
		write.dstSet = Set;
		write.dstBinding = BindingId;
		write.dstArrayElement = ArrayId;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
	}

	// TODO: REMOVE
	struct StagingBufferVulkan
	{
		VkBuffer Buffer;
		VmaAllocation Allocation;
	};

	// TODO: REMOVE
	StagingBufferVulkan CreateStagingBufferVulkanInternal(VmaAllocator allocator, size_t size, size_t align, const void* data)
	{
		StagingBufferVulkan buffer;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size + align;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		vmaallocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VK_CHECK(vmaCreateBufferWithAlignment(allocator, &bufferInfo, &vmaallocInfo, align,
			&buffer.Buffer,
			&buffer.Allocation,
			nullptr));

		if (data != nullptr)
		{
			void* mapped;
			VK_CHECK(vmaMapMemory(allocator, buffer.Allocation, &mapped));
			memcpy(mapped, data, size);
			vmaUnmapMemory(allocator, buffer.Allocation);
		}

		return buffer;
	}

	// TODO: REMOVE
	void DestroyStagingBufferVulkanInternal(VmaAllocator allocator, StagingBufferVulkan& buf)
	{
		vmaDestroyBuffer(allocator, buf.Buffer, buf.Allocation);
	}

	Buffer* DeviceVulkan::CreateBuffer(const BufferDesc& Desc, const void* InitialData)
	{
		// TODO: Enhance buffer creation
		auto result = new BufferVulkan(Desc);
		StagingBufferVulkan staging;

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = Desc.Size + Desc.Alignment;

		// TODO: enhance
		if (Desc.HostVisible)
		{
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}
		else
		{
			// TODO
			bufferInfo.usage = BufferTypeToVK(Desc.BindFlags) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		if (Desc.BindFlags == BufferType::Constant)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}

		if (InitialData != nullptr)
		{
			if (!Desc.HostVisible)
				staging = CreateStagingBufferVulkanInternal(_Allocator, Desc.Size, Desc.Alignment, InitialData);

			bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		}

		//if (deviceAddres.BindFlags)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		if (Desc.UsedInAccelerationStructure)
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		}

		VmaAllocationCreateInfo vmaallocInfo = {};

		if (Desc.HostVisible)
		{
			vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
			vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
			vmaallocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		}
		else
		{
			vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		VK_CHECK(vmaCreateBufferWithAlignment(_Allocator, &bufferInfo, &vmaallocInfo, Desc.Alignment,
			&result->_Buffer,
			&result->_Allocation,
			nullptr));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(_Allocator, result->_Allocation, &allocationInfo);

		result->SetSize(allocationInfo.size);

		if (!Desc.HostVisible)
		{
			VkBufferDeviceAddressInfo Info;
			Info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			Info.pNext = nullptr;
			Info.buffer = result->_Buffer;
			VkDeviceAddress Address = vkGetBufferDeviceAddress(_Device, &Info);
			result->SetDeviceAddress((u64)Address);
		}

		AddProfilingMemory(MemoryCounter_Vulkan_AllocatedBuffers, allocationInfo.size);
		if (Desc.HostVisible)
		{
			AddProfilingMemory(MemoryCounter_Vulkan_AllocatedHostVisibleBuffers, allocationInfo.size);
		}
		AddProfilingCount(CountingCounter_Vulkan_Buffers, 1);

		if (InitialData != nullptr)
		{
			if (Desc.HostVisible)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Host-visible buffer creation with initial data isn't supported");
			}
			else
			{
				auto copyCmdBuf = CreateCommandBufferShared();
				copyCmdBuf->Begin();
				VkBufferCopy copy = vk::BufferCopy(0, 0, Desc.Size + Desc.Alignment);
				vkCmdCopyBuffer(copyCmdBuf->_CmdBuf, staging.Buffer, result->_Buffer, 1, &copy);
				copyCmdBuf->End();

				Submit(copyCmdBuf.get());
				QueueWaitIdle();

				DestroyStagingBufferVulkanInternal(_Allocator, staging);
			}
		}

		return result;
	}

	void DeviceVulkan::DestroyBuffer(Buffer* Buf)
	{
		if (Buf == nullptr)
			return;

		auto vkbuf = static_cast<BufferVulkan*>(Buf);
		COLUMBUS_ASSERT(vkbuf);

		RemoveProfilingMemory(MemoryCounter_Vulkan_AllocatedBuffers, Buf->GetSize());
		if (Buf->GetDesc().HostVisible)
		{
			RemoveProfilingMemory(MemoryCounter_Vulkan_AllocatedHostVisibleBuffers, Buf->GetSize());
		}
		RemoveProfilingCount(CountingCounter_Vulkan_Buffers, 1);

		vmaDestroyBuffer(_Allocator, vkbuf->_Buffer, vkbuf->_Allocation);
	}

	void DeviceVulkan::DestroyBufferDeferred(Buffer* Buf)
	{
		BufferDeferredDestroys.push_back(ResourceDeferredDestroyVulkan<Buffer*> {
			.Resource = Buf,
			.FramesLasted = 0,
		});
	}

	void* DeviceVulkan::MapBuffer(const Buffer* Buf)
	{
		COLUMBUS_ASSERT_MESSAGE(Buf->GetDesc().HostVisible, "MapBuffer is available only for host-visible buffers");

		const BufferVulkan* vkbuf = static_cast<const BufferVulkan*>(Buf);
		void* Memory = nullptr;
		VK_CHECK(vmaMapMemory(_Allocator, vkbuf->_Allocation, &Memory));
		return Memory;
	}

	void DeviceVulkan::UnmapBuffer(const Buffer* Buf)
	{
		COLUMBUS_ASSERT_MESSAGE(Buf->GetDesc().HostVisible, "UnmapBuffer is available only for host-visible buffers");

		const BufferVulkan* vkbuf = static_cast<const BufferVulkan*>(Buf);
		vmaUnmapMemory(_Allocator, vkbuf->_Allocation);
	}

	constexpr size_t Align(size_t value, size_t alignment)
	{
		return (value + alignment - 1) & ~(alignment - 1);
	}

	Buffer* DeviceVulkan::GetConstantBufferPrepared(u32 Size, void* Data)
	{
		auto& FrameBuffers = _CBufPool.Pool[_CBufPool.CurrentFrame];

		// Round size up to alignment
		const u32 AlignedSize = Align(Size, 256);

		// Try to reuse an existing unused buffer of same size
		for (auto& entry : FrameBuffers)
		{
			if (!entry.Used && entry.Size == AlignedSize)
			{
				void* MappedPtr = MapBuffer(entry.Buf);
				memcpy(MappedPtr, Data, Size);
				UnmapBuffer(entry.Buf);
				entry.Used = true;
				return entry.Buf;
			}
		}

		// Allocate a new one
		BufferDesc CBDesc(AlignedSize, BufferType::Constant);
		CBDesc.HostVisible = true;

		Buffer* NewBuf = CreateBuffer(CBDesc, nullptr);
		void* MappedPtr = MapBuffer(NewBuf);
		memcpy(MappedPtr, Data, Size);
		UnmapBuffer(NewBuf);

		AddProfilingCount(CountingCounter_Vulkan_CBuffers, 1);

		FrameBuffers.push_back({ NewBuf, AlignedSize, true });
		return NewBuf;
	}

	// TODO: move all this logic to command buffer
	void TransitionImageLayout(VkCommandBuffer cmdbuf, VkImage image,
		VkImageLayout oldLayout, VkImageLayout newLayout,
		VkAccessFlags srcAccessMaks, VkAccessFlags dstAccessMask,
		VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, int layers, TextureFormat format)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(format);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layers;
		barrier.srcAccessMask = srcAccessMaks;
		barrier.dstAccessMask = dstAccessMask;
		vkCmdPipelineBarrier(
			cmdbuf,
			srcStageMask, dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	Texture2* DeviceVulkan::CreateTexture(const TextureDesc2& Desc)
	{
		auto result = _CreateTexture(Desc);

		auto copyCmdBuf = CreateCommandBufferShared();
		copyCmdBuf->Reset();
		copyCmdBuf->Begin();

		VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		switch (Desc.Usage)
		{
		case TextureUsage::Sampled:
			newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		case TextureUsage::Storage:
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case TextureUsage::StorageSampled:
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case TextureUsage::RenderTargetColor:
			newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		case TextureUsage::RenderTargetDepth:
			newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		case TextureUsage::StorageSampled | TextureUsage::RenderTargetColor:
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		case TextureUsage::Sampled | TextureUsage::RenderTargetColor:
		case TextureUsage::Sampled | TextureUsage::RenderTargetDepth:
			newLayout = VK_IMAGE_LAYOUT_GENERAL;
			break;
		default: COLUMBUS_ASSERT(false);
		};

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
				result->_Image, result->_Layout, newLayout,
			0, 0,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			1, Desc.Format
		);

		result->_Layout = newLayout;

		copyCmdBuf->End();
		Submit(copyCmdBuf.get());
		QueueWaitIdle();

		return result;
	}

	Texture2* DeviceVulkan::CreateTexture(const Image& Image)
	{
		TextureDesc2 desc;
		desc.Type = TextureType::Texture2D;
		desc.Usage = TextureUsage::Sampled;
		desc.Width = Image.GetWidth();
		desc.Height = Image.GetHeight();
		// desc.Depth = Image.GetDepth();
		desc.Depth = 1;
		desc.Mips = 1;
		desc.Samples = 1;
		desc.Format = Image.GetFormat();
		auto result = _CreateTexture(desc);

		UploadTextureMipData(result, 0, 0, Image.GetData());

		return result;
	}

	void DeviceVulkan::UploadTextureMipData(Texture2* Tex, int Mip, int Layer, const void* Data)
	{
		TextureVulkan* vktex = static_cast<TextureVulkan*>(Tex);
		TextureDesc2 Desc = Tex->GetDesc();

		// TODO: support async transfer

		// TODO:
		// not implemented
		assert(Layer == 0);
		assert(Mip == 0);

		Image Img; // dummy image for size calculations
		Img.Width = Desc.Width;
		Img.Height = Desc.Height;
		Img.Depth = Desc.Depth;
		Img.Format = Desc.Format;
		Img.MipMaps = Desc.Mips;

		int size = 1; // number of layers, TODO: fix this

		StagingBufferVulkan staging;
		staging = CreateStagingBufferVulkanInternal(_Allocator, Img.GetSize(0) * size, 0, Data);

		auto copyCmdBuf = CreateCommandBufferShared();
		copyCmdBuf->Reset();
		copyCmdBuf->Begin();

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
			vktex->_Image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0, VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			size, Desc.Format
		);

		VkBufferImageCopy regions[6];

		for (int i = 0; i < size; i++)
		{
			regions[i].bufferOffset = Img.GetOffset(i, 0);
			regions[i].bufferRowLength = 0;
			regions[i].bufferImageHeight = 0;
			regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			regions[i].imageSubresource.mipLevel = 0;
			regions[i].imageSubresource.baseArrayLayer = i;
			regions[i].imageSubresource.layerCount = 1;
			regions[i].imageOffset = { 0, 0, 0 };
			regions[i].imageExtent = { Img.GetWidth(), Img.GetHeight(), 1 };
		}

		vkCmdCopyBufferToImage(copyCmdBuf->_CmdBuf,
			staging.Buffer,
			vktex->_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			size,
			regions
		);

		TransitionImageLayout(copyCmdBuf->_CmdBuf,
			vktex->_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			size, Desc.Format
		);
		vktex->_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		copyCmdBuf->End();
		Submit(copyCmdBuf.get());
		QueueWaitIdle();

		DestroyStagingBufferVulkanInternal(_Allocator, staging);
	}

	void UploadTextureData(Texture2* Tex, int Mip, int NumMips, const void* Data)
	{
	}

	void DeviceVulkan::DestroyTexture(Texture2* Tex)
	{
		if (Tex == nullptr)
			return;

		auto vktex = static_cast<TextureVulkan*>(Tex);
		COLUMBUS_ASSERT(vktex);

		RemoveProfilingMemory(MemoryCounter_Vulkan_AllocatedImages, Tex->GetSize());
		RemoveProfilingCount(CountingCounter_Vulkan_Images, 1);

		vkDestroySampler(_Device, vktex->_Sampler, nullptr);
		vkDestroyImageView(_Device, vktex->_View, nullptr);
		if (vktex->_DepthView != NULL) vkDestroyImageView(_Device, vktex->_DepthView, nullptr);
		if (vktex->_StencilView != NULL) vkDestroyImageView(_Device, vktex->_StencilView, nullptr);
		vmaDestroyImage(_Allocator, vktex->_Image, vktex->_Allocation);
	}

	void DeviceVulkan::DestroyTextureDeferred(Texture2* Tex)
	{
		TextureDeferredDestroys.push_back(ResourceDeferredDestroyVulkan <Texture2*> {
			.Resource = Tex,
			.FramesLasted = 0,
		});
	}

	Sampler* DeviceVulkan::CreateSampler(const SamplerDesc& Desc)
	{
		SamplerVulkan* Result = new SamplerVulkan(Desc);

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = TextureFilterToVk(Desc.MagFilter);
		samplerInfo.minFilter = TextureFilterToVk(Desc.MinFilter);
		samplerInfo.mipmapMode = TextureFilterMipToVk(Desc.MipFilter);
		samplerInfo.addressModeU = TextureAddressModeToVk(Desc.AddressU);
		samplerInfo.addressModeV = TextureAddressModeToVk(Desc.AddressV);
		samplerInfo.addressModeW = TextureAddressModeToVk(Desc.AddressW);
		samplerInfo.mipLodBias = Desc.LodBias;
		samplerInfo.anisotropyEnable = Desc.Anisotropy > 1;
		samplerInfo.maxAnisotropy = Desc.Anisotropy;
		samplerInfo.compareEnable = false; // TODO
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // TODO
		samplerInfo.minLod = Desc.MinLOD;
		samplerInfo.maxLod = Desc.MaxLOD;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		VK_CHECK(vkCreateSampler(_Device, &samplerInfo, nullptr, &Result->_Sampler));

		return Result;
	}

	void DeviceVulkan::DestroySampler(Sampler* Sam)
	{
		if (Sam == nullptr)
			return;

		auto vksam = static_cast<SamplerVulkan*>(Sam);
		vkDestroySampler(_Device, vksam->_Sampler, nullptr);

		delete Sam;
	}

	Sampler* DeviceVulkan::GetStaticSampler(const SamplerDesc& Desc)
	{
		if (!StaticSamplers.contains(Desc))
		{
			StaticSamplers[Desc] = CreateSampler(Desc);
		}
		
		return StaticSamplers[Desc];
	}

	void DeviceVulkan::SetDebugName(const CommandBufferVulkan* CmdBuf, const char* Name)
	{
		_SetDebugName((uint64_t)CmdBuf->_CmdBuf, VK_OBJECT_TYPE_COMMAND_BUFFER, Name);
	}
	void DeviceVulkan::SetDebugName(const ComputePipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const ComputePipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const GraphicsPipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const GraphicsPipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const RayTracingPipeline* Pipeline, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const RayTracingPipelineVulkan*>(Pipeline)->pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
	}
	void DeviceVulkan::SetDebugName(const Buffer* Buffer, const char* Name)
	{
		_SetDebugName((uint64_t)static_cast<const BufferVulkan*>(Buffer)->_Buffer, VK_OBJECT_TYPE_BUFFER, Name);
	}
	void DeviceVulkan::SetDebugName(const Texture2* Texture, const char* Name)
	{
		auto vktex = static_cast<const TextureVulkan*>(Texture);
		_SetDebugName((uint64_t)vktex->_Image, VK_OBJECT_TYPE_IMAGE, Name);
		_SetDebugName((uint64_t)vktex->_View, VK_OBJECT_TYPE_IMAGE_VIEW, Name);
		_SetDebugName((uint64_t)vktex->_Sampler, VK_OBJECT_TYPE_SAMPLER, Name);
	}
	void DeviceVulkan::SetDebugName(const AccelerationStructure* AccelerationStructure, const char* Name)
	{
		if (SupportsRayTracing())
		{
			_SetDebugName((uint64_t)static_cast<const AccelerationStructureVulkan*>(AccelerationStructure)->_Handle, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR, Name);
		}
	}

	QueryPool* DeviceVulkan::CreateQueryPool(const QueryPoolDesc& Desc)
	{
		QueryPoolVulkan* Pool = new QueryPoolVulkan(Desc);

		VkQueryPoolCreateInfo Info;
		Info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		Info.pNext = nullptr;
		Info.flags = 0;
		Info.queryType = QueryPoolTypeToVk(Desc.Type);
		Info.queryCount = Desc.Count;
		Info.pipelineStatistics = 0;

		VK_CHECK(vkCreateQueryPool(_Device, &Info, nullptr, &Pool->_Pool));

		return Pool;
	}

	void DeviceVulkan::DestroyQueryPool(QueryPool* Pool)
	{
		if (Pool == nullptr)
			return;

		vkDestroyQueryPool(_Device, static_cast<QueryPoolVulkan*>(Pool)->_Pool, nullptr);

		delete Pool;
	}

	// void DeviceVulkan::ResetQueryPool(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount)
	// {
	// 	const QueryPoolVulkan* vkPool = static_cast<const QueryPoolVulkan*>(Pool);

	// 	vkResetQueryPool(_Device, vkPool->_Pool, FirstQuery, QueryCount);
	// }

	void DeviceVulkan::ReadQueryPoolTimestamps(const QueryPool* Pool, u32 FirstQuery, u32 QueryCount, u64* Data, u32 DataSize)
	{
		const QueryPoolVulkan* vkPool = static_cast<const QueryPoolVulkan*>(Pool);

		VK_CHECK(vkGetQueryPoolResults(_Device, vkPool->_Pool, FirstQuery, QueryCount, DataSize, Data, sizeof(u64), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));
	}

	template <typename T>
	static void RunDeferredDestroyLogic(std::vector<ResourceDeferredDestroyVulkan<T>>& DeferredDestroys, DeviceVulkan* Device, std::function<void(DeviceVulkan*, T)> DestroyFunc)
	{
		for (int i = 0; i < (int)DeferredDestroys.size(); i++)
		{
			auto& Destroy = DeferredDestroys[i];
			Destroy.FramesLasted++;
			if (Destroy.FramesLasted > MaxFramesInFlight)
			{
				DestroyFunc(Device, Destroy.Resource);
				DeferredDestroys.erase(DeferredDestroys.begin() + i);
				i--;
			}
		}
	}

	void DeviceVulkan::BeginFrame()
	{
		_Profiler.BeginFrame();
		_CBufPool.BeginFrame();
		UploadRing.BeginFrame();

		// run deferred destroy logic

		RunDeferredDestroyLogic<Buffer*>(BufferDeferredDestroys, this, [](DeviceVulkan* Dev, Buffer* Buf) {
			Dev->DestroyBuffer(Buf);
		});
		RunDeferredDestroyLogic<Texture2*>(TextureDeferredDestroys, this, [](DeviceVulkan* Dev, Texture2* Tex) {
			Dev->DestroyTexture(Tex);
		});
		RunDeferredDestroyLogic<VkRenderPass>(RenderPassDeferredDestroys, this, [](DeviceVulkan* Dev, VkRenderPass Pass) {
			vkDestroyRenderPass(Dev->_Device, Pass, nullptr);
		});
		RunDeferredDestroyLogic<VkFramebuffer>(FramebufferDeferredDestroys, this, [](DeviceVulkan* Dev, VkFramebuffer Framebuffer) {
			vkDestroyFramebuffer(Dev->_Device, Framebuffer, nullptr);
		});
	}

	void DeviceVulkan::EndFrame()
	{
		_Profiler.EndFrame();
	}


	void DeviceVulkan::WaitForFence(SPtr<FenceVulkan> fence, uint64_t timeout)
	{
		PROFILE_CPU(CpuCounter_Vulkan_FenceWaitTime);

		vkWaitForFences(_Device, 1, &fence->_Fence, true, timeout);
	}

	void DeviceVulkan::ResetFence(SPtr<FenceVulkan> fence)
	{
		vkResetFences(_Device, 1, &fence->_Fence);
	}

	bool DeviceVulkan::AcqureNextImage(SwapchainVulkan* swapchain, VkSemaphore signalSemaphore, uint32_t& imageIndex)
	{
		PROFILE_CPU(CpuCounter_Vulkan_AcquireImageTime);

		VkResult Result = vkAcquireNextImageKHR(_Device, swapchain->swapChain, UINT64_MAX, signalSemaphore, nullptr, &imageIndex);

		if (Result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return false;
		}

		VK_CHECK(Result);
		return true;
	}

	void DeviceVulkan::Submit(CommandBufferVulkan* Buffer, SPtr<FenceVulkan> fence, uint32_t waitSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t signalSemaphoresCount, VkSemaphore* signalSemaphores)
	{
		PROFILE_CPU(CpuCounter_Vulkan_SubmitTime);

		VkPipelineStageFlags waitStages[] = {
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		};

		VkSubmitInfo submit_info;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = waitSemaphoresCount;
		submit_info.pWaitSemaphores = waitSemaphores;
		submit_info.pWaitDstStageMask = waitStages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &Buffer->_GetHandle();
		submit_info.signalSemaphoreCount = signalSemaphoresCount;
		submit_info.pSignalSemaphores = signalSemaphores;

		if (fence)
		{
			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, fence->_Fence));
		}
		else
		{
			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, VK_NULL_HANDLE));
		}
	}

	void DeviceVulkan::Submit(CommandBufferVulkan* Buffer)
	{
		PROFILE_CPU(CpuCounter_Vulkan_SubmitTime);

		VkSubmitInfo submit_info;
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 0;
		submit_info.pWaitSemaphores = nullptr;
		submit_info.pWaitDstStageMask = nullptr;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &Buffer->_CmdBuf;
		submit_info.signalSemaphoreCount = 0;
		submit_info.pSignalSemaphores = nullptr;

		VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL));
	}

	void DeviceVulkan::QueueWaitIdle()
	{
		PROFILE_CPU(CpuCounter_Vulkan_QueueWaitIdleTime);

		VK_CHECK(vkQueueWaitIdle(*_ComputeQueue));
	}

	void DeviceVulkan::Present(SwapchainVulkan* swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		PROFILE_CPU(CpuCounter_Vulkan_PresentTime);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &waitSemaphore;

		VkSwapchainKHR swapChains[] = { swapchain->swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = &imageIndex;

		if (vkQueuePresentKHR(*_ComputeQueue, &presentInfo) != VK_SUCCESS)
		{
			swapchain->IsOutdated = true;
		}
	}

	VkPipelineLayout DeviceVulkan::_CreatePipelineLayout(const CompiledShaderData& Bytecode, PipelineDescriptorSetLayoutsVulkan& OutSetLayouts)
	{
		// TODO: Caching scheme
		for (int i = 0; i < Bytecode.Reflection->DescriptorSets.size(); i++)
		{
			auto& setInfo = Bytecode.Reflection->DescriptorSets[i];

			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo;
			bindingFlagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsInfo.pNext = nullptr;
			bindingFlagsInfo.bindingCount = setInfo.BindingFlags.size();
			bindingFlagsInfo.pBindingFlags = setInfo.BindingFlags.data();

			VkDescriptorSetLayoutCreateInfo setLayoutInfo;
			setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setLayoutInfo.pNext = nullptr;
			setLayoutInfo.flags = 0;
			setLayoutInfo.bindingCount = setInfo.Bindings.size();
			setLayoutInfo.pBindings = setInfo.Bindings.data();

			OutSetLayouts.VariableCountMax[i] = setInfo.VariableCountMax;
			if (setInfo.VariableCountMax > 0)
			{
				setLayoutInfo.pNext = &bindingFlagsInfo;
			}

			OutSetLayouts.UsedLayouts++;
			VK_CHECK(vkCreateDescriptorSetLayout(_Device, &setLayoutInfo, nullptr, &OutSetLayouts.Layouts[i]));

			AddProfilingCount(CountingCounter_Vulkan_PipelineLayouts, 1);
		}

		VkPipelineLayoutCreateInfo layoutInfo;
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.setLayoutCount = Bytecode.Reflection->DescriptorSets.size();
		layoutInfo.pSetLayouts = OutSetLayouts.Layouts;
		layoutInfo.pushConstantRangeCount = Bytecode.Reflection->PushConstants.size();
		layoutInfo.pPushConstantRanges = Bytecode.Reflection->PushConstants.data();

		VkPipelineLayout Layout;
		VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &Layout));

		return Layout;
	}

	VkDescriptorSet DeviceVulkan::_CreateDescriptorSet(const PipelineDescriptorSetLayoutsVulkan& SetLayouts, int Index)
	{
		// only for variable descriptor counts
		VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorSetCounts;
		descriptorSetCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
		descriptorSetCounts.pNext = nullptr;
		descriptorSetCounts.descriptorSetCount = 1;
		descriptorSetCounts.pDescriptorCounts = &SetLayouts.VariableCountMax[Index];

		VkDescriptorSetAllocateInfo descriptorSetInfo;
		descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetInfo.pNext = nullptr;
		descriptorSetInfo.descriptorPool = _DescriptorPool;
		descriptorSetInfo.descriptorSetCount = 1;
		descriptorSetInfo.pSetLayouts = &SetLayouts.Layouts[Index];

		if (SetLayouts.VariableCountMax[Index] > 0)
		{
			descriptorSetInfo.pNext = &descriptorSetCounts;
		}

		VkDescriptorSet result;

		AddProfilingCount(CountingCounter_Vulkan_DescriptorSets, 1);

		VK_CHECK(vkAllocateDescriptorSets(_Device, &descriptorSetInfo, &result));

		return result;
	}

	void DeviceVulkan::_SetDebugName(uint64_t ObjectHandle, VkObjectType Type, const char* Name)
	{
		if (IsVulkanDebugEnabled())
		{
			VkDebugUtilsObjectNameInfoEXT nameInfo;
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.pNext = nullptr;
			nameInfo.objectType = Type;
			nameInfo.objectHandle = ObjectHandle;
			nameInfo.pObjectName = Name;
			VK_CHECK(VkFunctions.vkSetDebugUtilsObjectName(_Device, &nameInfo));
		}
	}

	TextureVulkan* DeviceVulkan::_CreateTexture(const TextureDesc2& Desc)
	{
		auto result = new TextureVulkan(Desc);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = NULL;
		imageInfo.flags = TextureTypeToImageFlagsVk(Desc.Type);
		imageInfo.imageType = TextureTypeToImageTypeVk(Desc.Type);
		imageInfo.format = TextureFormatToVK(Desc.Format);
		imageInfo.extent.width = Desc.Width;
		imageInfo.extent.height = Desc.Height;
		imageInfo.extent.depth = Desc.Depth;
		imageInfo.mipLevels = Desc.Mips;
		imageInfo.arrayLayers = Desc.ArrayLayers;
		imageInfo.samples = SampleCountToVk(Desc.Samples);
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = TextureUsageToImageUsageVk(Desc.Usage) | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // TODO
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.pQueueFamilyIndices = &_FamilyIndex;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VK_CHECK(vmaCreateImage(_Allocator, &imageInfo, &vmaallocInfo, &result->_Image, &result->_Allocation, nullptr));

		VmaAllocationInfo allocationInfo;
		vmaGetAllocationInfo(_Allocator, result->_Allocation, &allocationInfo);
		result->SetSize(allocationInfo.size);

		AddProfilingMemory(MemoryCounter_Vulkan_AllocatedImages, allocationInfo.size);
		AddProfilingCount(CountingCounter_Vulkan_Images, 1);

		result->_Layout = VK_IMAGE_LAYOUT_UNDEFINED;

		// colour or depth-stencil view
		{
			VkImageViewCreateInfo viewInfo;
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.pNext = nullptr;
			viewInfo.flags = 0;
			viewInfo.image = result->_Image;
			viewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
			viewInfo.format = TextureFormatToVK(Desc.Format);
			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
			viewInfo.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(Desc.Format); // colour or depth-stencil
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = Desc.Mips;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
			VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &result->_View));
		}

		TextureFormatInfo FormatInfo = TextureFormatGetInfo(Desc.Format);

		if (FormatInfo.HasDepth)
		{
			// depth-only view
			{
				VkImageViewCreateInfo viewInfo;
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.pNext = nullptr;
				viewInfo.flags = 0;
				viewInfo.image = result->_Image;
				viewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
				viewInfo.format = TextureFormatToVK(Desc.Format);
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = Desc.Mips;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
				VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &result->_DepthView));
			}

			if (FormatInfo.HasStencil)
			{
				// stencil-only view
				VkImageViewCreateInfo viewInfo;
				viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewInfo.pNext = nullptr;
				viewInfo.flags = 0;
				viewInfo.image = result->_Image;
				viewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
				viewInfo.format = TextureFormatToVK(Desc.Format);
				viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // TODO
				viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
				viewInfo.subresourceRange.baseMipLevel = 0;
				viewInfo.subresourceRange.levelCount = Desc.Mips;
				viewInfo.subresourceRange.baseArrayLayer = 0;
				viewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
				VK_CHECK(vkCreateImageView(_Device, &viewInfo, nullptr, &result->_StencilView));
			}
		}

		VkSamplerCreateInfo samplerInfo;
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.pNext = nullptr;
		samplerInfo.flags = 0;
		samplerInfo.magFilter = TextureFilterToVk(Desc.MagFilter);
		samplerInfo.minFilter = TextureFilterToVk(Desc.MinFilter);
		samplerInfo.mipmapMode = TextureFilterMipToVk(Desc.MipFilter);
		samplerInfo.addressModeU = TextureAddressModeToVk(Desc.AddressU);
		samplerInfo.addressModeV = TextureAddressModeToVk(Desc.AddressV);
		samplerInfo.addressModeW = TextureAddressModeToVk(Desc.AddressW);
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = Desc.Anisotropy > 1;
		samplerInfo.maxAnisotropy = Desc.Anisotropy;
		samplerInfo.compareEnable = false; // TODO
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // TODO
		samplerInfo.minLod = Desc.MinLOD;
		samplerInfo.maxLod = Desc.MaxLOD;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		VK_CHECK(vkCreateSampler(_Device, &samplerInfo, nullptr, &result->_Sampler));

		return result;
	}

}
