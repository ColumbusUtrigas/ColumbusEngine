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

#define VULKAN_DEBUG 1

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

		SPtr<DeviceVulkan> CreateDevice()
		{
			// enumerate physical devices
			uint32_t count;
			if (vkEnumeratePhysicalDevices(instance, &count, nullptr) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to enumerate Vulkan physical devices");
			}

			std::vector<VkPhysicalDevice> devices(count);
			if (vkEnumeratePhysicalDevices(instance, &count, devices.data()) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to enumerate Vulkan physical devices");
			}

			// TODO: choose the most powerful device
			return SPtr<DeviceVulkan>(new DeviceVulkan(devices[0], instance));
		}

		~InstanceVulkan()
		{
			#ifdef VULKAN_DEBUG
				vk_vkDestroyDebugUtilsMessengerEXT(instance, _DebugUtilsMessenger, nullptr);
			#endif

			vkDestroyInstance(instance, nullptr);
		}
	};

}
