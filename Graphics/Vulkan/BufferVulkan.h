#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <Lib/VulkanMemoryAllocator/include/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class BufferVulkan : public Buffer
	{
	public:
		VkBuffer _Buffer;
		VmaAllocation _Allocation;
	public:
		BufferVulkan(const BufferDesc& Desc) : Buffer(Desc) {}

		// TODO: REMOVE
		virtual void* GetHandle() override { return nullptr; }

		void SetSize(u64 ByteSize)
		{
			Size = ByteSize;
		}
	};

}
