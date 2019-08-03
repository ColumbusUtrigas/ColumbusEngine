#pragma once

#include <vulkan/vulkan.h>
#include <Core/Types.h>

namespace Columbus
{

	class CommandBufferVulkan
	{
	private:
		VkCommandBuffer _CmdBuf;
	public:
		CommandBufferVulkan(VkCommandBuffer CmdBuf) :
			_CmdBuf(CmdBuf) {}

		void Begin();
		void End();

		//void BindDescriptorSets();
		//void BindPipeline();
		void Dispatch(uint32 X, uint32 Y, uint32 Z);

		const VkCommandBuffer& _GetHandle() const { return _CmdBuf; }
	};

}


