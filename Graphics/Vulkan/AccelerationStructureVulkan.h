#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class AccelerationStructureVulkan : public AccelerationStructure
	{
	public:
		VkAccelerationStructureKHR _Handle;
		Buffer* _Buffer;
		uint64_t _DeviceAddress;

	public:
		AccelerationStructureVulkan(const AccelerationStructureDesc& Desc) : AccelerationStructure(Desc) {}
	};

}
