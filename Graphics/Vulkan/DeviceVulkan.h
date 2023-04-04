#pragma once

#include "Core/fixed_vector.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/Types.h"
#include "Graphics/Vulkan/TypeConversions.h"
#include "VulkanMemoryAllocator/include/vk_mem_alloc.h"
#include "include/spirv/unified1/spirv.h"
#include <Core/Assert.h>
#include <Core/SmartPointer.h>

#include <Graphics/Device.h>
#include <Graphics/Vulkan/SwapchainVulkan.h>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Graphics/Vulkan/GraphicsPipelineVulkan.h>
#include <Graphics/Vulkan/BufferVulkan.h>
#include <Core/Types.h>

#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <Lib/SPIRV-Reflect/spirv_reflect.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>
#include <cassert>

#include <Common/Image/Image.h>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	struct BindingVulkan
	{
		VkDescriptorSetLayoutBinding _Binding;

		BindingVulkan(uint32_t Binding, VkDescriptorType Type, uint32_t Count)
		{
			_Binding.binding = Binding;
			_Binding.descriptorType = Type;
			_Binding.descriptorCount = Count;
			_Binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT; // TODO
			_Binding.pImmutableSamplers = nullptr;
		}

		operator VkDescriptorSetLayoutBinding() const
		{
			return _Binding;
		}
	};

	struct TextureVulkan
	{
		VkImage image;
		VkImageView view;
		VkSampler sampler;
		VmaAllocation allocation;
	};

	/*namespace Graphics::Vulkan
	{
		class DeviceVulkan : public Device
		{
			virtual ShaderProgram* CreateShaderProgram() const override;
			virtual Texture* CreateTexture() const override;
			virtual Mesh* CreateMesh() const override;
			virtual Framebuffer* CreateFramebuffer() const override;

			virtual void IASetPrimitiveTopology(PrimitiveTopology Topology) final override;
			virtual void IASetInputLayout(InputLayout* Layout) final override;
			virtual void IASetVertexBuffers(uint32 StartSlot, uint32 NumBuffers, Buffer** ppBuffers) final override;
			virtual void IASetIndexBuffer(Buffer* pIndexBuffer, IndexFormat Format, uint32 Offset) final override;

			virtual void OMSetBlendState(BlendState* pBlendState, const float BlendFactor[4], uint32 SampleMask) final override;
			virtual void OMSetDepthStencilState(DepthStencilState* pDepthStencilState, uint32 StencilRef) final override;

			virtual void RSSetState(RasterizerState* pRasterizerState) final override;

			virtual void SetShader(ShaderProgram* Prog) final override;

			virtual void SetComputePipelineState(ComputePipelineState* State) final override;

			virtual bool CreateBlendState(const BlendStateDesc& Desc, BlendState** ppBlendState) final override;
			virtual bool CreateDepthStencilState(const DepthStencilStateDesc& Desc, DepthStencilState** ppDepthStencilState) final override;
			virtual bool CreateRasterizerState(const RasterizerStateDesc& Desc, RasterizerState** ppRasterizerState) final override;

			virtual bool CreateBuffer(const BufferDesc& Desc, SubresourceData* pInitialData, Buffer** ppBuffer) final override;
			virtual void BindBufferBase(Buffer* pBuffer, uint32 Index) final override;
			virtual void BindBufferRange(Buffer* pBuffer, uint32 Index, uint32 Offset, uint32 Size) final override;
			virtual void MapBuffer(Buffer* pBuffer, BufferMapAccess MapAccess, void*& MappedData) final override;
			virtual void UnmapBuffer(Buffer* pBuffer) final override;

			virtual bool CreateComputePipelineState(const ComputePipelineStateDesc& Desc, ComputePipelineState** ppComputePipelineState) final override;

			virtual void Dispatch(uint32 X, uint32 Y, uint32 Z) final override;
			virtual void Draw(uint32 VertexCount, uint32 StartVertexLocation) final override;
			virtual void DrawIndexed(uint32 IndexCount, uint32 StartIndexLocation, int BaseVertexLocation) final override;

			virtual void BeginMarker(const char* Str) final override;
			virtual void EndMarker() final override;
		};
	}*/

	/**Represents device (GPU) on which Vulkan is executed.*/
	class DeviceVulkan
	{
	public:
		VkPhysicalDevice _PhysicalDevice;
		VkDevice _Device;

		VkPhysicalDeviceProperties _DeviceProperties;
		VkPhysicalDeviceFeatures _DeviceFeatures;
		VkPhysicalDeviceMemoryProperties _MemoryProperties;

		uint32 _FamilyIndex; // TODO: multiple families, now supports only graphics
		SmartPointer<VkQueue> _ComputeQueue; // TODO: multiple queues

		VkCommandPool _CmdPool;
		VkDescriptorPool _DescriptorPool;

		VmaAllocator _Allocator;
	public:
		DeviceVulkan(VkPhysicalDevice PhysicalDevice, VkInstance Instance) :
			_PhysicalDevice(PhysicalDevice)
		{
			vkGetPhysicalDeviceProperties(PhysicalDevice, &_DeviceProperties);
			vkGetPhysicalDeviceFeatures(PhysicalDevice, &_DeviceFeatures);
			vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &_MemoryProperties);

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
			queueCreateInfo.queueCount = 1; // I don't really know what it actually is and how it to use
			queueCreateInfo.pQueuePriorities = &queuePriorities;

			std::vector<const char*> extensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			VkDeviceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.queueCreateInfoCount = 1;
			info.pQueueCreateInfos = &queueCreateInfo;
			info.enabledLayerCount = 0;
			info.ppEnabledLayerNames = nullptr;
			info.enabledExtensionCount = extensions.size();
			info.ppEnabledExtensionNames = extensions.data();
			info.pEnabledFeatures = nullptr;

			if (vkCreateDevice(PhysicalDevice, &info, nullptr, &_Device) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan logical device");
			}

			_ComputeQueue = SmartPointer<VkQueue>(new VkQueue);
			vkGetDeviceQueue(_Device, _FamilyIndex, 0, _ComputeQueue.Get());

			// create buffer pool
			VkCommandPoolCreateInfo commandPoolInfo;
			commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPoolInfo.pNext = nullptr;
			commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPoolInfo.queueFamilyIndex = _FamilyIndex;

			if (vkCreateCommandPool(_Device, &commandPoolInfo, nullptr, &_CmdPool) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan command pool");
			}

			// create descriptor pool
			VkDescriptorPoolSize poolSizes[2];
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSizes[0].descriptorCount = 100;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 100;

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.flags = 0;
			descriptorPoolInfo.maxSets = 1024;
			descriptorPoolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
			descriptorPoolInfo.pPoolSizes = poolSizes;

			if (vkCreateDescriptorPool(_Device, &descriptorPoolInfo, nullptr, &_DescriptorPool) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan descriptor pool");
			}

			// initialize VMA
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = PhysicalDevice;
			allocatorInfo.device = _Device;
			allocatorInfo.instance = Instance;
			vmaCreateAllocator(&allocatorInfo, &_Allocator);
		}

		SwapchainVulkan* CreateSwapchain(VkSurfaceKHR surface);
		VkRenderPass CreateRenderPass(VkFormat format);
		void CreateFramebuffers(SwapchainVulkan* swapchain, VkRenderPass renderpass);

		CommandBufferVulkan CreateCommandBuffer()
		{
			VkCommandBufferAllocateInfo commandBufferInfo;
			commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			commandBufferInfo.pNext = nullptr;
			commandBufferInfo.commandPool = _CmdPool;
			commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			commandBufferInfo.commandBufferCount = 1;

			VkCommandBuffer result;
			if (vkAllocateCommandBuffers(_Device, &commandBufferInfo, &result) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to allocate Vulkan command buffer");
			}
			return result;
		}

		VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<BindingVulkan>& Bindings)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(Bindings.begin(), Bindings.end());

			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
			descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutInfo.pNext = nullptr;
			descriptorSetLayoutInfo.flags = 0;
			descriptorSetLayoutInfo.bindingCount = bindings.size();
			descriptorSetLayoutInfo.pBindings = bindings.data();

			VkDescriptorSetLayout descriptorSetLayout;
			if (vkCreateDescriptorSetLayout(_Device, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan descriptor set layout");
			}
			return descriptorSetLayout;
		}

		VkPipeline CreateComputePipeline(VkPipelineLayout PipelineLayout)
		{
			// read file
			std::ifstream f("vkCompute.spv");
			std::vector<char> code;
			std::copy(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>(),
				std::back_inserter(code));

			// create shader module
			VkShaderModuleCreateInfo moduleInfo;
			moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			moduleInfo.pNext = nullptr;
			moduleInfo.flags = 0;
			moduleInfo.codeSize = code.size();
			moduleInfo.pCode = (uint32*)code.data();
			
			VkShaderModule module;
			if (vkCreateShaderModule(_Device, &moduleInfo, nullptr, &module) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan shader module");
			}

			// shader stage info
			VkPipelineShaderStageCreateInfo stage;
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.pNext = nullptr;
			stage.flags = 0;
			stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			stage.module = module;
			stage.pName = "main";
			stage.pSpecializationInfo = nullptr;

			// create pipeline
			VkComputePipelineCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.stage = stage;
			info.layout = PipelineLayout;
			info.basePipelineHandle = nullptr;
			info.basePipelineIndex = -1;

			VkPipeline result;
			if (vkCreateComputePipelines(_Device, nullptr, 1, &info, nullptr, &result) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan compute pipeline");
			}
			return result;
		}

		struct ShaderStageCompilationResult
		{
			VkPipelineShaderStageCreateInfo shaderStageInfo;
			fixed_vector<VkPushConstantRange, 16> pushConstants; // TODO
			fixed_vector<VkDescriptorSetLayoutCreateInfo, 16> descriptorSets; // TODO
		};

		ShaderStageCompilationResult BuildShaderStage(SPtr<Columbus::ShaderStage> stage)
		{
			ShaderStageCompilationResult result;
			shaderc::Compiler compiler;
			shaderc_shader_kind kind;
			shaderc::CompileOptions options;
			VkShaderStageFlagBits vkstage;

			switch (stage->Type)
			{
				case ShaderType::Vertex:
					kind = shaderc_vertex_shader;
					options.AddMacroDefinition("VERTEX_SHADER");
					vkstage = VK_SHADER_STAGE_VERTEX_BIT;
					break;

				case ShaderType::Pixel:
					kind = shaderc_fragment_shader;
					options.AddMacroDefinition("PIXEL_SHADER");
					vkstage = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;

				case ShaderType::Compute:
					kind = shaderc_compute_shader;
					options.AddMacroDefinition("COMPUTE_SHADER");
					vkstage = VK_SHADER_STAGE_COMPUTE_BIT;
					break;

				default: break;
			}

			auto spirv = compiler.CompileGlslToSpv(stage->Source, kind, "name", options);
			printf("Compilation error: %s\n", spirv.GetErrorMessage().c_str());

			// Generate reflection data for a shader
			SpvReflectShaderModule spv_module;
			SpvReflectResult reflect_result = spvReflectCreateShaderModule((spirv.end() - spirv.begin()) * 4, spirv.begin(), &spv_module);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			// Enumerate and extract shader's input variables
			uint32_t input_count = 0;
			SpvReflectInterfaceVariable* inputs[128];

			reflect_result = spvReflectEnumerateInputVariables(&spv_module, &input_count, NULL);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			reflect_result = spvReflectEnumerateInputVariables(&spv_module, &input_count, inputs);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			// Enumerate and extract shader's push constants
			uint32_t push_constant_num = 0;
			SpvReflectBlockVariable* push_constants[128];

			reflect_result = spvReflectEnumeratePushConstantBlocks(&spv_module, &push_constant_num, NULL);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			reflect_result = spvReflectEnumeratePushConstantBlocks(&spv_module, &push_constant_num, push_constants);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			// Enumerate and extract shader's descriptor sets
			uint32_t sets_num = 0;
			SpvReflectDescriptorSet* sets[128];

			spvReflectEnumerateDescriptorSets(&spv_module, &sets_num, NULL);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			spvReflectEnumerateDescriptorSets(&spv_module, &sets_num, sets);
			assert(reflect_result == SPV_REFLECT_RESULT_SUCCESS);

			// for (int i = 0; i < input_count; i++)
			// {
			// 	SpvReflectInterfaceVariable* input = inputs[i];
			// 	printf("%s, location(%i), storage(%i), decoration(%x), type(vec%i)\n", var->name, var->location, var->storage_class, var->decoration_flags, var->numeric.vector.component_count);
			// }

			for (int i = 0; i < push_constant_num; i++)
			{
				assert(stage->Type == ShaderType::Vertex);

				VkPushConstantRange push;
				push.stageFlags = vkstage;
				push.offset = push_constants[i]->offset;
				push.size = push_constants[i]->size;

				result.pushConstants.push_back(push);
			}			

			for (int i = 0; i < sets_num; i++)
			{
				auto bindings = new fixed_vector<VkDescriptorSetLayoutBinding, 16>(); // TODO, MEMORY LEAK
				for (int b = 0; b < sets[i]->binding_count; b++)
				{
					auto binding = sets[i]->bindings[b];
					assert(stage->Type == ShaderType::Pixel);
					assert(binding->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

					VkDescriptorSetLayoutBinding bindingInfo;
					bindingInfo.binding = binding->binding;
					bindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					bindingInfo.descriptorCount = binding->count;
					bindingInfo.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
					bindingInfo.pImmutableSamplers = nullptr;

					bindings->push_back(bindingInfo);
				}

				VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
				descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
				descriptorSetLayoutInfo.pNext = nullptr;
				descriptorSetLayoutInfo.flags = 0;
				descriptorSetLayoutInfo.bindingCount = bindings->size();
				descriptorSetLayoutInfo.pBindings = bindings->data();

				result.descriptorSets.push_back(descriptorSetLayoutInfo);
			}

			spvReflectDestroyShaderModule(&spv_module);

			VkShaderModuleCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.codeSize = (spirv.end() - spirv.begin()) * 4;
			info.pCode = spirv.begin();

			VkShaderModule module;

			vkCreateShaderModule(_Device, &info, nullptr, &module);

			result.shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			result.shaderStageInfo.pNext = nullptr;
		    result.shaderStageInfo.flags = 0;
			result.shaderStageInfo.stage = vkstage;
			result.shaderStageInfo.module = module;
			result.shaderStageInfo.pName = stage->EntryPoint.c_str();
			result.shaderStageInfo.pSpecializationInfo = nullptr;

			return result;
		}

		Graphics::GraphicsPipeline* CreateGraphicsPipeline(const Graphics::GraphicsPipelineDesc& desc, VkRenderPass renderPass)
		{
			auto vs = BuildShaderStage(desc.VS);
			auto ps = BuildShaderStage(desc.PS);
			VkPipelineShaderStageCreateInfo stages[] = { vs.shaderStageInfo, ps.shaderStageInfo };

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

			// VkDescriptorSetLayoutBinding textureBinding;
			// textureBinding.binding = 0;
			// textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			// textureBinding.descriptorCount = 1;
			// textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			// textureBinding.pImmutableSamplers = nullptr;

			// VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
			// descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			// descriptorSetLayoutInfo.pNext = nullptr;
			// descriptorSetLayoutInfo.flags = 0;
			// descriptorSetLayoutInfo.bindingCount = 1;
			// descriptorSetLayoutInfo.pBindings = &textureBinding;

			for (int i = 0; i < ps.descriptorSets.size(); i++)
			{
				vkCreateDescriptorSetLayout(_Device, &ps.descriptorSets[i], nullptr, &pipeline->setLayouts[i]);
			}

			VkPipelineLayoutCreateInfo layoutInfo;
			layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutInfo.pNext = nullptr;
			layoutInfo.flags = 0;
			layoutInfo.setLayoutCount = ps.descriptorSets.size();
			layoutInfo.pSetLayouts = pipeline->setLayouts;
			layoutInfo.pushConstantRangeCount = vs.pushConstants.size();
			layoutInfo.pPushConstantRanges = vs.pushConstants.data();

			if (vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &pipeline->layout) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan pipeline layout")
			}

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

			if (vkCreateGraphicsPipelines(_Device, nullptr, 1, &info, nullptr, &pipeline->pipeline) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan graphics pipeline")
			}

			return pipeline;
		}

		VkDescriptorSet CreateDescriptorSet(VkDescriptorSetLayout DescriptorSetLayout)
		{
			VkDescriptorSetAllocateInfo descriptorSetInfo;
			descriptorSetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetInfo.pNext = nullptr;
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

		VkFence CreateFence(bool signaled)
		{
			VkFenceCreateInfo fenceInfo;
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = signaled  ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
			fenceInfo.pNext = nullptr;

			VkFence fence;
			if (vkCreateFence(_Device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan Fence");
			}
			return fence;
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

		void WaitForFences(uint32_t fenceCount, const VkFence* pFences, bool waitAll, uint64_t timeout)
		{
			vkWaitForFences(_Device, fenceCount, pFences, waitAll, timeout);
		}

		void ResetFences(uint32_t fenceCount, const VkFence* pFences)
		{
			vkResetFences(_Device, fenceCount, pFences);
		}

		void UpdateDescriptorSet(VkDescriptorSet DescriptorSet, TextureVulkan texture)
		{
			VkDescriptorImageInfo imageInfo;
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture.view;
			imageInfo.sampler = texture.sampler;

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
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.pImageInfo = &imageInfo;
			write.pBufferInfo = nullptr;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
		}

		void AcqureNextImage(SwapchainVulkan* swapchain, VkSemaphore signalSemaphore, uint32_t& imageIndex)
		{
			vkAcquireNextImageKHR(_Device, swapchain->swapChain, UINT64_MAX, signalSemaphore, nullptr, &imageIndex);
		}

		void Submit(CommandBufferVulkan Buffer, VkFence fence, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore)
		{
			VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

			VkSubmitInfo submit_info;
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = nullptr;
			submit_info.waitSemaphoreCount = 1;
			submit_info.pWaitSemaphores = &waitSemaphore;
			submit_info.pWaitDstStageMask = waitStages;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &Buffer._GetHandle();
			submit_info.signalSemaphoreCount = 1;
			submit_info.pSignalSemaphores = &signalSemaphore;

			vkQueueSubmit(*_ComputeQueue, 1, &submit_info, fence);
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

		BufferVulkan CreateBuffer(size_t Size, const void* Data, BufferType type);
		TextureVulkan CreateTexture(const Image& image);

		// ~DeviceVulkan()
		// {
		// 	VkResult result = vkDeviceWaitIdle(_Device);
		// 	assert(result == VK_SUCCESS);

		// 	vkDestroyDevice(_Device, nullptr);
		// }
	};

}
