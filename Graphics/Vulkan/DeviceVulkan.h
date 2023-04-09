#pragma once

#include "Core/fixed_vector.h"
#include "Graphics/ComputePipeline.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RayTracingPipeline.h"
#include "Graphics/Types.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/DeviceVulkanFunctions.h"
#include "Graphics/Vulkan/FenceVulkan.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "Graphics/Vulkan/VulkanShaderCompiler.h"
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include <Core/Assert.h>
#include <Core/SmartPointer.h>

#include <Graphics/Device.h>
#include <Graphics/Vulkan/SwapchainVulkan.h>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/GraphicsPipelineVulkan.h>
#include <Graphics/Vulkan/BufferVulkan.h>
#include <Core/Types.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <cassert>

#include <Common/Image/Image.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	struct TextureVulkan
	{
		VkDevice _Device;
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VmaAllocation allocation;

		~TextureVulkan()
		{
			vkDestroyImageView(_Device, view, nullptr);
			vkDestroySampler(_Device, sampler, nullptr);
			vkDestroyImage(_Device, image, nullptr);
		}
	};

	/**Represents device (GPU) on which Vulkan is executed.*/
	class DeviceVulkan
	{
	public:
		VkPhysicalDevice _PhysicalDevice;
		VkDevice _Device;

		VkPhysicalDeviceVulkan12Properties _Vulkan12Properties;
		VkPhysicalDeviceVulkan12Features _Vulkan12Features;

		VkPhysicalDeviceAccelerationStructurePropertiesKHR _AccelerationStructureProperties;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR _AccelerationStructureFeatures;

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR _RayTracingProperties;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR _RayTracingFeatures;

		VkPhysicalDeviceProperties2 _DeviceProperties;
		VkPhysicalDeviceFeatures2 _DeviceFeatures;
		VkPhysicalDeviceMemoryProperties _MemoryProperties;

		uint32 _FamilyIndex; // TODO: multiple families, now supports only graphics
		SmartPointer<VkQueue> _ComputeQueue; // TODO: multiple queues

		VkCommandPool _CmdPool;
		VkDescriptorPool _DescriptorPool;

		VmaAllocator _Allocator;

		VulkanFunctions VkFunctions;
	public:
		DeviceVulkan(VkPhysicalDevice PhysicalDevice, VkInstance Instance) :
			_PhysicalDevice(PhysicalDevice)
		{
			_Vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
			_Vulkan12Features.pNext = nullptr;
			_AccelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			_AccelerationStructureFeatures.pNext = &_Vulkan12Features;
			_RayTracingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			_RayTracingFeatures.pNext = &_AccelerationStructureFeatures;
			_DeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			_DeviceFeatures.pNext = &_RayTracingFeatures;

			vkGetPhysicalDeviceFeatures2(PhysicalDevice, &_DeviceFeatures);

			_RayTracingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
			_RayTracingProperties.pNext = nullptr;
			_DeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			_DeviceProperties.pNext = &_RayTracingProperties;

			vkGetPhysicalDeviceProperties2(PhysicalDevice, &_DeviceProperties);
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &_MemoryProperties);

			// logic if features are enabled/disabled
			COLUMBUS_ASSERT(_Vulkan12Features.descriptorIndexing); // non-uniform descriptor indexing
			COLUMBUS_ASSERT(_Vulkan12Features.descriptorBindingPartiallyBound);
			COLUMBUS_ASSERT(_Vulkan12Features.bufferDeviceAddress);
			COLUMBUS_ASSERT(_RayTracingFeatures.rayTracingPipeline);

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

				VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
				VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
				VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
				VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
			};

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

			// create descriptor pool
			VkDescriptorPoolSize poolSizes[5];
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[0].descriptorCount = 100;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 100;
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
			poolSizes[2].descriptorCount = 10;
			poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			poolSizes[3].descriptorCount = 10;
			poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[4].descriptorCount = 10;

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.flags = 0;
			descriptorPoolInfo.maxSets = 1024;
			descriptorPoolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
			descriptorPoolInfo.pPoolSizes = poolSizes;

			VK_CHECK(vkCreateDescriptorPool(_Device, &descriptorPoolInfo, nullptr, &_DescriptorPool));

			// initialize VMA
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = PhysicalDevice;
			allocatorInfo.device = _Device;
			allocatorInfo.instance = Instance;
			allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			vmaCreateAllocator(&allocatorInfo, &_Allocator);
		}

		SwapchainVulkan* CreateSwapchain(VkSurfaceKHR surface);
		VkRenderPass CreateRenderPass(VkFormat format);
		void CreateFramebuffers(SwapchainVulkan* swapchain, VkRenderPass renderpass);

		CommandBufferVulkan CreateCommandBuffer();

		uint64_t GetBufferDeviceAddress(BufferVulkan Buffer)
		{
			VkBufferDeviceAddressInfo info;
			info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			info.pNext = nullptr;
			info.buffer = Buffer.Buffer;

			return vkGetBufferDeviceAddress(_Device, &info);
		}

		struct ShaderStageCompilationResult
		{
			VkPipelineShaderStageCreateInfo ShaderStageInfo;
			CompiledSpirv Spirv;
		};

		ShaderStageCompilationResult BuildShaderStage(SPtr<Columbus::ShaderStage> stage, const std::string& name)
		{
			ShaderStageCompilationResult result;
			result.Spirv = CompileShaderStage_VK(stage, name);

			VkShaderModuleCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.codeSize = result.Spirv.Bytecode.size() * sizeof(uint32_t);
			info.pCode = result.Spirv.Bytecode.data();

			VkShaderModule module;

			VK_CHECK(vkCreateShaderModule(_Device, &info, nullptr, &module));

			result.ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			result.ShaderStageInfo.pNext = nullptr;
		    result.ShaderStageInfo.flags = 0;
			result.ShaderStageInfo.stage = ShaderTypeToVk(stage->Type);
			result.ShaderStageInfo.module = module;
			result.ShaderStageInfo.pName = stage->EntryPoint.c_str();
			result.ShaderStageInfo.pSpecializationInfo = nullptr;
			return result;
		}

		ComputePipeline* CreateComputePipeline(const ComputePipelineDesc& Desc);

		Graphics::GraphicsPipeline* CreateGraphicsPipeline(const Graphics::GraphicsPipelineDesc& desc, VkRenderPass renderPass)
		{
			auto vs = BuildShaderStage(desc.VS, desc.Name);
			auto ps = BuildShaderStage(desc.PS, desc.Name);

			std::vector<ShaderStageCompilationResult> compiledStages = { vs, ps };
			VkPipelineShaderStageCreateInfo stages[] = { vs.ShaderStageInfo, ps.ShaderStageInfo };

			auto pipeline = new Graphics::GraphicsPipelineVulkan(desc);

			fixed_vector<VkVertexInputBindingDescription, 16> inputBindings;
			for (int i = 0; i < desc.layout.Elements.size(); i++)
			{
				auto element = desc.layout.Elements[i];

				VkVertexInputBindingDescription vertexInputBinding;
				vertexInputBinding.binding = element.Slot;
				vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // TODO
				vertexInputBinding.stride = sizeof(float) * element.Components; // TODO

				inputBindings.push_back(vertexInputBinding);
			}

			// TODO
			fixed_vector<VkVertexInputAttributeDescription, 16> inputAttribute;
			for (int i = 0; i < desc.layout.Elements.size(); i++)
			{
				auto element = desc.layout.Elements[i];

				VkVertexInputAttributeDescription attribute;

				attribute.binding = element.Slot;
				attribute.location = element.SemanticIndex;
				attribute.offset = 0;

				// TODO
				switch (element.Components)
				{
				case 2: attribute.format = VK_FORMAT_R32G32_SFLOAT;    break;
				case 3: attribute.format = VK_FORMAT_R32G32B32_SFLOAT; break;
				}

				inputAttribute.push_back(attribute);
			}

			VkPipelineVertexInputStateCreateInfo vertexInputState;
			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.pNext = nullptr;
			vertexInputState.flags = 0;
			vertexInputState.vertexBindingDescriptionCount = inputBindings.size();
			vertexInputState.pVertexBindingDescriptions = inputBindings.data();
			vertexInputState.vertexAttributeDescriptionCount = inputAttribute.size();
			vertexInputState.pVertexAttributeDescriptions = inputAttribute.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.pNext = nullptr;
			inputAssemblyState.flags = 0;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssemblyState.primitiveRestartEnable = false;

			VkPipelineTessellationStateCreateInfo tesselationState;
			tesselationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tesselationState.pNext = nullptr;
			tesselationState.flags = 0;
			tesselationState.patchControlPoints = 1;

			VkViewport viewport;
			viewport.x = 0;
			viewport.y = 0;
			viewport.width = 1280;
			viewport.height = 720;
			viewport.minDepth = 0;
			viewport.maxDepth = 1;

			VkRect2D scissor;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			scissor.extent.width = 1280;
			scissor.extent.height = 720;

			VkPipelineViewportStateCreateInfo viewportState;
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;
			viewportState.flags = 0;
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &scissor;

			VkPipelineRasterizationStateCreateInfo rasterState;
			rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterState.pNext = nullptr;
			rasterState.flags = 0;
			rasterState.depthClampEnable = false;
			rasterState.rasterizerDiscardEnable =  false;
			rasterState.polygonMode = FillModeToVK(desc.rasterizerState.Fill);
			rasterState.cullMode = CullModeToVK(desc.rasterizerState.Cull);
			rasterState.frontFace = desc.rasterizerState.FrontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
			rasterState.depthBiasEnable = false;
			rasterState.depthBiasConstantFactor = desc.rasterizerState.DepthBias;
			rasterState.depthBiasClamp = desc.rasterizerState.DepthBiasClamp;
			rasterState.depthBiasSlopeFactor = desc.rasterizerState.SlopeScaledDepthBias;
			rasterState.lineWidth = 1;

			VkPipelineMultisampleStateCreateInfo multisampleState;
			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.pNext = nullptr;
			multisampleState.flags = 0;
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleState.sampleShadingEnable = false;
			multisampleState.minSampleShading = 0;
			multisampleState.pSampleMask = nullptr;
			multisampleState.alphaToCoverageEnable = false;
			multisampleState.alphaToOneEnable = false;

			VkPipelineDepthStencilStateCreateInfo depthStencilState;
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.pNext = nullptr;
			depthStencilState.flags = 0;
			depthStencilState.depthTestEnable = false;
			depthStencilState.depthWriteEnable = false;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState.stencilTestEnable = false;
			depthStencilState.front = {};
			depthStencilState.back = {};
			depthStencilState.minDepthBounds = 0;
			depthStencilState.maxDepthBounds = 0;

			VkPipelineColorBlendAttachmentState attachment;
			attachment.blendEnable = false;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

			VkPipelineColorBlendStateCreateInfo blendState;
			blendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			blendState.pNext = nullptr;
			blendState.flags = 0;
			blendState.logicOpEnable = false;
			blendState.logicOp = VK_LOGIC_OP_COPY;
			blendState.attachmentCount = 1;
			blendState.pAttachments = &attachment;
			blendState.blendConstants[0] = 0;
			blendState.blendConstants[1] = 0;
			blendState.blendConstants[2] = 0;
			blendState.blendConstants[3] = 0;

			VkPipelineDynamicStateCreateInfo dynamicState;
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pNext = nullptr;
			dynamicState.flags = 0;
			dynamicState.dynamicStateCount = 0;
			dynamicState.pDynamicStates = nullptr;

			// Concat all push constants and descriptor sets
			std::vector<VkPushConstantRange> pushConstants;
			std::vector<VkDescriptorSetLayoutCreateInfo> descriptorSets;

			for (auto& stage : compiledStages)
			{
				for (auto& pushConstant : stage.Spirv.pushConstants)
				{
					pushConstants.push_back(pushConstant);
				}

				for (auto& descriptorSet : stage.Spirv.descriptorSets)
				{
					descriptorSets.push_back(descriptorSet);
				}
			}

			for (int i = 0; i < descriptorSets.size(); i++)
			{
				VK_CHECK(vkCreateDescriptorSetLayout(_Device, &descriptorSets[i], nullptr, &pipeline->setLayouts[i]));
			}

			VkPipelineLayoutCreateInfo layoutInfo;
			layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutInfo.pNext = nullptr;
			layoutInfo.flags = 0;
			layoutInfo.setLayoutCount = descriptorSets.size();
			layoutInfo.pSetLayouts = pipeline->setLayouts;
			layoutInfo.pushConstantRangeCount = pushConstants.size();
			layoutInfo.pPushConstantRanges = pushConstants.data();

			VK_CHECK(vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &pipeline->layout));

			VkGraphicsPipelineCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.stageCount = 2;
			info.pStages = stages;
			info.pVertexInputState = &vertexInputState;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pTessellationState = &tesselationState;
			info.pViewportState = &viewportState;
			info.pRasterizationState = &rasterState;
			info.pMultisampleState = &multisampleState;
			info.pDepthStencilState = &depthStencilState;
			info.pColorBlendState = &blendState;
			info.pDynamicState = &dynamicState;
			info.layout = pipeline->layout;
			info.renderPass = renderPass;
			info.subpass = 0;
			info.basePipelineHandle = nullptr;
			info.basePipelineIndex = 0;

			VK_CHECK(vkCreateGraphicsPipelines(_Device, nullptr, 1, &info, nullptr, &pipeline->pipeline));

			return pipeline;
		}

		RayTracingPipeline* CreateRayTracingPipeline(const RayTracingPipelineDesc& Desc);

		uint32_t alignedSize(uint32_t value, uint32_t alignment)
        {
	        return (value + alignment - 1) & ~(alignment - 1);
        }

		uint32_t getHandleSizeAligned()
		{
			return alignedSize(_RayTracingProperties.shaderGroupHandleSize, _RayTracingProperties.shaderGroupHandleAlignment);
		}

		VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout DescriptorSetLayout, bool variableCount = false)
		{
			uint32_t counts[] = { 100 }; // TODO

			VkDescriptorSetVariableDescriptorCountAllocateInfo descriptorSetCounts;
			descriptorSetCounts.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
			descriptorSetCounts.pNext = nullptr;
			descriptorSetCounts.descriptorSetCount = 1;
			descriptorSetCounts.pDescriptorCounts = counts;

			VkDescriptorSetAllocateInfo descriptorSetInfo;
			descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetInfo.pNext = variableCount ? &descriptorSetCounts : nullptr;
			descriptorSetInfo.descriptorPool = _DescriptorPool;
			descriptorSetInfo.descriptorSetCount = 1;
			descriptorSetInfo.pSetLayouts = &DescriptorSetLayout;

			VkDescriptorSet descriptorSet;
			if (vkAllocateDescriptorSets(_Device, &descriptorSetInfo, &descriptorSet) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan descriptor set");
			}
			return descriptorSet;
		}

		SPtr<FenceVulkan> CreateFence(bool signaled)
		{
			return std::make_shared<FenceVulkan>(_Device, signaled);
		}

		VkSemaphore CreateSemaphore()
		{
			VkSemaphoreCreateInfo semaphoreInfo;
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreInfo.pNext = nullptr;
			semaphoreInfo.flags = 0;

			VkSemaphore semaphore;
			if (vkCreateSemaphore(_Device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan Semaphore")
			}

			return semaphore;
		}

		void WaitForFence(SPtr<FenceVulkan> fence, uint64_t timeout)
		{
			vkWaitForFences(_Device, 1, &fence->_Fence, true, timeout);
		}

		void ResetFence(SPtr<FenceVulkan> fence)
		{
			vkResetFences(_Device, 1, &fence->_Fence);
		}

		void UpdateDescriptorSet(VkDescriptorSet DescriptorSet, SPtr<TextureVulkan> texture, bool storage = false)
		{
			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->view;
			imageInfo.sampler = texture->sampler;

			// VkDescriptorBufferInfo bufferInfo;
			// bufferInfo.buffer = Buffer;
			// bufferInfo.offset = 0;
			// bufferInfo.range = size;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = DescriptorSet;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = storage ? VK_DESCRIPTOR_TYPE_STORAGE_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &imageInfo;
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
		}

		void UpdateRtDescriptorSet(VkDescriptorSet set, VkAccelerationStructureKHR tlas, VkImageView image)
		{
			VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
			descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
			descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
			descriptorAccelerationStructureInfo.pAccelerationStructures = &tlas;

			VkWriteDescriptorSet accelerationStructureWrite{};
			accelerationStructureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			// The specialized acceleration structure descriptor has to be chained
			accelerationStructureWrite.pNext = &descriptorAccelerationStructureInfo;
			accelerationStructureWrite.dstSet = set;
			accelerationStructureWrite.dstBinding = 0;
			accelerationStructureWrite.descriptorCount = 1;
			accelerationStructureWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

			VkDescriptorImageInfo storageImageDescriptor{};
			storageImageDescriptor.imageView = image;
			storageImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			VkWriteDescriptorSet resultImageWrite{};
			resultImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			resultImageWrite.pNext = nullptr;
			resultImageWrite.dstSet = set;
			resultImageWrite.dstBinding = 1;
			resultImageWrite.descriptorCount = 1;
			resultImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			resultImageWrite.pImageInfo = &storageImageDescriptor;

			std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
				accelerationStructureWrite,
				resultImageWrite
			};

			vkUpdateDescriptorSets(_Device, 2, writeDescriptorSets.data(), 0, nullptr);
		}

		void UpdateArrayDescriptorSet(VkDescriptorSet set, uint32_t start, VkBuffer buffer)
		{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.dstSet = set;
			write.dstBinding = 0;
			write.descriptorCount = 1;
			write.dstArrayElement = start;
			write.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
		}

		void UpdateArrayDescriptorSet(VkDescriptorSet set, uint32_t start, VkImageView image, VkSampler sampler)
		{
			VkDescriptorImageInfo imageDescriptor{};
			imageDescriptor.imageView = image;
			imageDescriptor.sampler = sampler;
			imageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstSet = set;
			write.dstBinding = 0;
			write.descriptorCount = 1;
			write.dstArrayElement = start;
			write.pImageInfo = &imageDescriptor;

			vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
		}


		void AcqureNextImage(SwapchainVulkan* swapchain, VkSemaphore signalSemaphore, uint32_t& imageIndex)
		{
			vkAcquireNextImageKHR(_Device, swapchain->swapChain, UINT64_MAX, signalSemaphore, nullptr, &imageIndex);
		}

		void Submit(const CommandBufferVulkan& Buffer, SPtr<FenceVulkan> fence, uint32_t waitSemaphoresCount, VkSemaphore* waitSemaphores, uint32_t signalSemaphoresCount, VkSemaphore* signalSemaphores)
		{
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
			submit_info.pCommandBuffers = &Buffer._GetHandle();
			submit_info.signalSemaphoreCount = signalSemaphoresCount;
			submit_info.pSignalSemaphores = signalSemaphores;

			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, fence->_Fence));
		}

		void Submit(const CommandBufferVulkan& Buffer)
		{
			VkSubmitInfo submit_info;
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = nullptr;
			submit_info.waitSemaphoreCount = 0;
			submit_info.pWaitSemaphores = nullptr;
			submit_info.pWaitDstStageMask = nullptr;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &Buffer._CmdBuf;
			submit_info.signalSemaphoreCount = 0;
			submit_info.pSignalSemaphores = nullptr;

			VK_CHECK(vkQueueSubmit(*_ComputeQueue, 1, &submit_info, NULL));
		}

		void QueueWaitIdle()
		{
			VK_CHECK(vkQueueWaitIdle(*_ComputeQueue));
		}

		void Present(SwapchainVulkan* swapchain, uint32_t imageIndex, VkSemaphore waitSemaphore)
		{
			VkPresentInfoKHR presentInfo{};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &waitSemaphore;

			VkSwapchainKHR swapChains[] = { swapchain->swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;

			presentInfo.pImageIndices = &imageIndex;

			vkQueuePresentKHR(*_ComputeQueue, &presentInfo);
		}

		BufferVulkan CreateBuffer(size_t Size, const void* Data, BufferType type, bool deviceAddress = false, bool asInput = false);
		SPtr<TextureVulkan> CreateTexture(const Image& image);
		SPtr<TextureVulkan> CreateStorageImage();

		AccelerationStructureVulkan* CreateAccelerationStructure(const AccelerationStructureDesc& Desc);

		void DestroyBuffer(BufferVulkan buffer)
		{
			vmaDestroyBuffer(_Allocator, buffer.Buffer, buffer.Allocation);
		}

		~DeviceVulkan()
		{
			// VK_CHECK(vkDeviceWaitIdle(_Device));

			// vmaDestroyAllocator(_Allocator);
			// vkDestroyDescriptorPool(_Device, _DescriptorPool, nullptr);
			// vkDestroyCommandPool(_Device, _CmdPool, nullptr);
			// vkDestroyDevice(_Device, nullptr);
		}
	};

}
