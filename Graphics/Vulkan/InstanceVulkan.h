#pragma once

#include <vulkan/vulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>
#include <Core/Assert.h>
#include <Core/SmartPointer.h>
#include <Core/Platform.h>
#include <vector>
#include <iostream>
#include <vulkan/vulkan_core.h>

namespace Columbus
{

	class InstanceVulkan
	{
	public:
		VkInstance instance;
	public:
		InstanceVulkan()
		{
			VkApplicationInfo appInfo;
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = nullptr;
			appInfo.pApplicationName = "Engine";
			appInfo.applicationVersion = 001;
			appInfo.pEngineName = "Columbus Engine";
			appInfo.engineVersion = 030;
			appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);

			std::vector<const char*> validationLayers;
			std::vector<const char*> extensions;

			#if 1 // ENGINE_DEBUG, my engine is in permanent debug :D
				//validationLayers.push_back("VK_LAYER_LUNARG_standard_validation");
				validationLayers.push_back("VK_LAYER_KHRONOS_validation");
				extensions.push_back("VK_EXT_debug_report");
			#endif

			extensions.push_back("VK_KHR_surface");

			#ifdef PLATFORM_LINUX
			extensions.push_back("VK_KHR_xlib_surface");
			#elif PLATFORM_WINDOWS
			extensions.push_back("VK_KHR_win32_surface");
			#endif

			VkInstanceCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
			info.pApplicationInfo = &appInfo;
			info.enabledLayerCount = validationLayers.size();
			info.ppEnabledLayerNames = validationLayers.data();
			info.enabledExtensionCount = extensions.size();
			info.ppEnabledExtensionNames = extensions.data();

			if (vkCreateInstance(&info, nullptr, &instance) != VK_SUCCESS)
			{
				COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan instance");
			}

			// setup validation layers
			#if 1 // ENGINE_DEBUG
				auto vk_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
  					vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");

  				VkDebugReportCallbackEXT debug_callback;
				VkDebugReportCallbackCreateInfoEXT debug_callback_info;

				debug_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
				debug_callback_info.pNext = nullptr;
				debug_callback_info.flags =
				//VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_DEBUG_BIT_EXT;

				debug_callback_info.pfnCallback = [](
					VkDebugReportFlagsEXT    flags,
					VkDebugReportObjectTypeEXT    objectType,
					uint64_t        object,
					size_t          location,
					int32_t         messageCode,
					const char*     pLayerPrefix,
					const char*     pMessage,
					void*           pUserData) -> VkBool32
				{
					std::cout << "(";
					if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) std::cout << "INFO";
					if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) std::cout << "WARNING";
					if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) std::cout << "PERFORMANCE";
					if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) std::cout << "DEBUG";
					if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) std::cout << "ERROR";
					std::cout << ")";
					std::cout << "{" << pLayerPrefix << "} " << pMessage << std::endl;
					return VK_FALSE;
				};

				debug_callback_info.pUserData = nullptr;

				if(vk_vkCreateDebugReportCallbackEXT(instance,
					&debug_callback_info,
					nullptr,
					&debug_callback) != VK_SUCCESS)
				{
					COLUMBUS_ASSERT_MESSAGE(false, "Failed to create Vulkan debug callback");
				}
			#endif
		}

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

		// void _test()
		// {
		// 	uint cpuBuffer[1] = {2};
		// 	VkBuffer gpuBuffer = _Device->CreateBuffer(sizeof(cpuBuffer), cpuBuffer);

		// 	printf("Data before: %u\n", cpuBuffer[0]);

		// 	VkDescriptorSetLayout setLayout = _Device->CreateDescriptorSetLayout({
		// 		{ 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 }
		// 	});
		// 	VkPipelineLayout pipelineLayout = _Device->CreatePipelineLayout(setLayout);
		// 	VkPipeline pipeline = _Device->CreateComputePipeline(pipelineLayout);
		// 	VkDescriptorSet set = _Device->CreateDescriptorSet(setLayout);

		// 	CommandBufferVulkan cmdBuf = _Device->CreateCommandBuffer();

		// 	_Device->UpdateDescriptorSet(set, gpuBuffer, sizeof(cpuBuffer));

		// 	cmdBuf.Begin();
		// 	cmdBuf.BindDescriptorSet(set, pipelineLayout);
		// 	cmdBuf.BindPipeline(pipeline);
		// 	cmdBuf.Dispatch(1, 1, 1);
		// 	cmdBuf.End();
		// 	_Device->Submit(cmdBuf);
		// }

		~InstanceVulkan()
		{
			vkDestroyInstance(instance, nullptr);
		}
	};

}
