#pragma once

#include <Graphics/Core/GraphicsCore.h>
#include <vulkan/vulkan.h>

namespace Columbus
{

	class AccelerationStructureVulkan : public AccelerationStructure
	{
	public:
		VkAccelerationStructureKHR _Handle = 0;
		Buffer* _Buffer = nullptr;
		Buffer* _ScratchBuffer = nullptr;
		Buffer* _TransformOrInstancesBuffer = nullptr;

		// TODO: better uploading system
		Buffer* _InstancesUploadBuffers[3]{ nullptr };
		int CurrentUploadId = 0;

		uint64_t _DeviceAddress = 0;

	public:
		AccelerationStructureVulkan(const AccelerationStructureDesc& Desc) : AccelerationStructure(Desc) {}

		void SetSize(u64 SizeBytes)
		{
			Size = SizeBytes;
		}
	};

}
