#pragma once

#include <Core/Assert.h>
#include <Core/SmartPointer.h>

#include <Graphics/Device.h>>
#include <Graphics/Vulkan/CommandBufferVulkan.h>
#include <Core/Types.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>
#include <cassert>

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
	private:
		VkPhysicalDevice _PhysicalDevice;
		VkDevice _Device;

		VkPhysicalDeviceProperties _DeviceProperties;
		VkPhysicalDeviceFeatures _DeviceFeatures;
		VkPhysicalDeviceMemoryProperties _MemoryProperties;

		uint32 _FamilyIndex; // TODO: multiple families, now supports only compute
		SmartPointer<VkQueue> _ComputeQueue; // TODO: multiple queues

		VkCommandPool _CmdPool;
		VkDescriptorPool _DescriptorPool;
	public:
		DeviceVulkan(VkPhysicalDevice PhysicalDevice) :
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
				if (qFamProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) // supports compute
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

			VkDeviceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.queueCreateInfoCount = 1;
			info.pQueueCreateInfos = &queueCreateInfo;
			info.enabledLayerCount = 0;
			info.ppEnabledLayerNames = nullptr;
			info.enabledExtensionCount = 0;
			info.ppEnabledExtensionNames = nullptr;
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
			VkDescriptorPoolSize poolSize;
			poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSize.descriptorCount = 100;

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			descriptorPoolInfo.sType;
			descriptorPoolInfo.pNext = nullptr;
			descriptorPoolInfo.flags;
			descriptorPoolInfo.maxSets = 1024;
			descriptorPoolInfo.poolSizeCount = 1;
			descriptorPoolInfo.pPoolSizes = &poolSize;

			if (vkCreateDescriptorPool(_Device, &descriptorPoolInfo, nullptr, &_DescriptorPool) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan descriptor pool");
			}
		}

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
			descriptorSetLayoutInfo.sType;
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

		VkPipelineLayout CreatePipelineLayout(VkDescriptorSetLayout DescriptorSetLayout)
		{
			VkPipelineLayoutCreateInfo layoutInfo;
			layoutInfo.sType;
			layoutInfo.pNext = nullptr;
			layoutInfo.flags = 0;
			layoutInfo.setLayoutCount = 1;
			layoutInfo.pSetLayouts = &DescriptorSetLayout;
			layoutInfo.pushConstantRangeCount = 0;
			layoutInfo.pPushConstantRanges = nullptr;

			VkPipelineLayout layout;
			if (vkCreatePipelineLayout(_Device, &layoutInfo, nullptr, &layout) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan pipeline layout");
			}
			return layout;
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

		void UpdateDescriptorSet(VkDescriptorSet DescriptorSet, VkBuffer Buffer, size_t size)
		{
			VkDescriptorBufferInfo bufferInfo;
			bufferInfo.buffer = Buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = size;

			VkWriteDescriptorSet write;
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.pNext = nullptr;
			write.dstSet = DescriptorSet;
			write.dstBinding = 0;
			write.dstArrayElement = 0;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.pImageInfo = nullptr;
			write.pBufferInfo = &bufferInfo;
			write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(_Device, 1, &write, 0, nullptr);
		}

		void Submit(CommandBufferVulkan Buffer)
		{
			VkFenceCreateInfo fenceInfo;
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = 0;

			VkFence fence;
			if (vkCreateFence(_Device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan fence");
			}

			VkSubmitInfo submit_info;
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.pNext = nullptr;
			submit_info.waitSemaphoreCount = 0;
			submit_info.pWaitSemaphores = nullptr;
			submit_info.pWaitDstStageMask = nullptr;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &Buffer._GetHandle();
			submit_info.signalSemaphoreCount = 0;
			submit_info.pSignalSemaphores = nullptr;

			vkQueueSubmit(*_ComputeQueue, 1, &submit_info, fence);
			if (vkWaitForFences(_Device, 1, &fence, VK_TRUE, UINT64_MAX) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to wait Vulkan fence");
			}

			uint32 data[1];
			void* mapped;
			if(vkMapMemory(_Device, mem, 0, sizeof(data), 0, &mapped) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to map Vulkan memory");
			}
			memcpy(data, mapped, sizeof(data));
			vkUnmapMemory(_Device, mem);

			printf("Data after: %u\n", data[0]);
		}

		VkDeviceMemory mem;

		VkBuffer CreateBuffer(size_t Size, const void* Data)
		{
			VkBufferCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.size = Size;
			info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;

			VkBuffer result;
			if (vkCreateBuffer(_Device, &info, nullptr, &result) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan buffer");
			}

			VkMemoryRequirements mem_req;
			vkGetBufferMemoryRequirements(_Device, result, &mem_req);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = Size;
			alloc_info.memoryTypeIndex = [&]() {
				for(size_t i = 0; i < _MemoryProperties.memoryTypeCount; ++i)
				{
					auto bit = ((uint32_t)1 << i);
					if((mem_req.memoryTypeBits & bit) != 0)
					{
						if (_MemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
						{
							return i;
						}
					}
				}

				COLUMBUS_ASSERT_MESSAGE(false, "Failed to get correct memory type");
				return size_t(-1);
			}();

			
			if (vkAllocateMemory(_Device, &alloc_info, nullptr, &mem) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to allocate Vulkan memory");
			}

			void* mapped;
			if(vkMapMemory(_Device, mem, 0, Size, 0, &mapped) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to map Vulkan memory");
			}
			memcpy(mapped, Data, Size);
			vkUnmapMemory(_Device, mem);

			if (vkBindBufferMemory(_Device, result, mem, 0) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to fill Vulkan buffer");
			}

			return result;
		}

		~DeviceVulkan()
		{
			VkResult result = vkDeviceWaitIdle(_Device);
			assert(result == VK_SUCCESS);

			vkDestroyDevice(_Device, nullptr);
		}
	};

}


