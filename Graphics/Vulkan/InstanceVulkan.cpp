#include "InstanceVulkan.h"

bool GVulkanDebugEnabled = false;

bool IsVulkanDebugEnabled()
{
	return GVulkanDebugEnabled;
}

namespace Columbus
{

    InstanceVulkan::InstanceVulkan()
    {
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "Engine";
		appInfo.applicationVersion = 001;
		appInfo.pEngineName = "Columbus Engine";
		appInfo.engineVersion = 030;
		appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 0);

		std::vector<const char*> validationLayers;
		std::vector<const char*> extensions;

#if VULKAN_DEBUG
		GVulkanDebugEnabled |= GCommandLine.HasArg("-vkdebug");
	#if !NDEBUG
		GVulkanDebugEnabled = true;
	#endif
#endif
		if (GCommandLine.HasArg("-novkdebug"))
		{
			GVulkanDebugEnabled = false;
		}

		if (IsVulkanDebugEnabled())
		{
			validationLayers.push_back("VK_LAYER_KHRONOS_validation");
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

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
		info.enabledLayerCount = (u32)validationLayers.size();
		info.ppEnabledLayerNames = validationLayers.data();
		info.enabledExtensionCount = (u32)extensions.size();
		info.ppEnabledExtensionNames = extensions.data();

		// GPU assisted validation
#if 0
		if (IsVulkanDebugEnabled())
		{
			VkValidationFeaturesEXT validationFeatures{};
			validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
			validationFeatures.enabledValidationFeatureCount = 1;

			VkValidationFeatureEnableEXT enables[] = {
				VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
			};

			validationFeatures.pEnabledValidationFeatures = enables;
			validationFeatures.pNext = nullptr;

			// When creating VkInstance
			info.pNext = &validationFeatures;
		}
#endif

		VK_CHECK(vkCreateInstance(&info, nullptr, &instance));

		Log::Initialization("Vulkan Instance created");
		Log::Initialization("Vulkan Validation layers are %s", IsVulkanDebugEnabled() ? "ON" : "OFF");
    }

	SPtr<DeviceVulkan> InstanceVulkan::CreateDevice()
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
		auto Result = SPtr<DeviceVulkan>(new DeviceVulkan(devices[0], instance));
		Log::Initialization("Vulkan Device created");

		// setup debug messages
		if (IsVulkanDebugEnabled())
		{
			vk_vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			vk_vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

			VkDebugUtilsMessengerCreateInfoEXT messengerInfo;
			messengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			messengerInfo.pNext = nullptr;
			messengerInfo.flags = 0;
			messengerInfo.messageSeverity =
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			messengerInfo.messageType =
				VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

			messengerInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
			messengerInfo.pUserData = nullptr;

			messengerInfo.pfnUserCallback = [](
				VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
				VkDebugUtilsMessageTypeFlagsEXT             types,
				const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
				void* pUserData) -> VkBool32
				{
					const char* prefix = "";
					if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) prefix = "GENERAL";
					if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) prefix = "VALIDATION";
					if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) prefix = "PERFORMANCE";
					if (types & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) prefix = "DEVICE_ADDRESS_BINDING";

					const char* message = pCallbackData->pMessage;
					const char* messageId = pCallbackData->pMessageIdName;

					if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) Log::Message("%s {%s} %s", prefix, messageId, message);
					if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) Log::Message("%s {%s} %s", prefix, messageId, message);
					if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) Log::Warning("%s {%s} %s", prefix, messageId, message);
					if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
					{
						Log::Error("%s {%s} %s", prefix, messageId, message);
						//WriteStacktraceToLog();
						DEBUGBREAK();
					}

					return VK_FALSE;
				};

			VK_CHECK(vk_vkCreateDebugUtilsMessengerEXT(instance, &messengerInfo, nullptr, &_DebugUtilsMessenger));
			Log::Initialization("Vulkan Validation callback was set up");
		}

		return Result;
	}

}
