#pragma once

#include "System/Log.h"
#include <vulkan/vulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <Core/Assert.h>
#include <Core/SmartPointer.h>
#include <Core/Platform.h>
#include <Core/Stacktrace.h>
#include "Common.h"
#include <vector>

namespace Columbus
{

	class InstanceVulkan
	{
	private:
		PFN_vkCreateDebugUtilsMessengerEXT vk_vkCreateDebugUtilsMessengerEXT;
		PFN_vkDestroyDebugUtilsMessengerEXT vk_vkDestroyDebugUtilsMessengerEXT;

		VkDebugUtilsMessengerEXT _DebugUtilsMessenger;

	public:
		VkInstance instance;
	public:
		InstanceVulkan();

		SPtr<DeviceVulkan> CreateDevice();

		~InstanceVulkan()
		{
			if (IsVulkanDebugEnabled())
			{
				vk_vkDestroyDebugUtilsMessengerEXT(instance, _DebugUtilsMessenger, nullptr);
			}

			vkDestroyInstance(instance, nullptr);
		}
	};

}
