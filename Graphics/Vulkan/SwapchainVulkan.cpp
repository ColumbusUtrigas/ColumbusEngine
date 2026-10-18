#include "SwapchainVulkan.h"

namespace Columbus
{

	SwapchainVulkan::SwapchainVulkan(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainVulkan* OldSwapchain)
	{
		auto supportDetails = QuerySwapChainSupport(physicalDevice, surface);
		auto surfaceFormat = ChooseSwapSurfaceFormat(supportDetails.formats);
		auto presentMode = ChooseSwapPresentMode(supportDetails.presentModes);
		auto extent = ChooseSwapExtent(supportDetails.capabilities);
		Device = device;

		minImageCount = supportDetails.capabilities.minImageCount;
		imageCount = supportDetails.capabilities.minImageCount + 1;
		if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
			imageCount = supportDetails.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		/*auto indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else */ {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform = supportDetails.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = OldSwapchain ? OldSwapchain->swapChain : VK_NULL_HANDLE;

		VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));

		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;

		CreateTextures(device);
	}

	void SwapchainVulkan::CreateTextures(VkDevice device)
	{
		TextureDesc2 Desc;
		Desc.Usage  = TextureUsage::RenderTargetColor;
		Desc.Width  = swapChainExtent.width;
		Desc.Height = swapChainExtent.height;

		switch (swapChainImageFormat)
		{
		case VK_FORMAT_B8G8R8A8_SRGB:  Desc.Format = TextureFormat::BGRA8SRGB; break;
		case VK_FORMAT_B8G8R8A8_UNORM: Desc.Format = TextureFormat::BGRA8; break;
		case VK_FORMAT_R8G8B8A8_SRGB:  Desc.Format = TextureFormat::RGBA8SRGB; break;
		case VK_FORMAT_R8G8B8A8_UNORM: Desc.Format = TextureFormat::RGBA8; break;
		default:
			COLUMBUS_ASSERT_MESSAGE(false, "Unsupported swapchain texture format");
			return;
		}

		for (VkImage Image : swapChainImages)
		{
			auto* Texture = new TextureVulkan(Desc);
			Texture->_Image = Image;
			Textures.push_back(Texture);
		}
	}

	void SwapchainVulkan::DestroyTextures()
	{
		for (Texture2* Texture : Textures)
		{
			auto* TextureVk = static_cast<TextureVulkan*>(Texture);
			for (TextureViewVulkan* View : TextureVk->_CachedViews)
			{
				vkDestroyImageView(Device, View->_View, nullptr);
				delete View;
			}
			delete TextureVk;
		}
		Textures.clear();
	}

	SwapchainVulkan::~SwapchainVulkan()
	{
		DestroyTextures();
		vkDestroySwapchainKHR(Device, swapChain, nullptr);
	}

	SwapChainSupportDetails SwapchainVulkan::QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR SwapchainVulkan::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapchainVulkan::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		return VK_PRESENT_MODE_IMMEDIATE_KHR;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D SwapchainVulkan::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		return capabilities.currentExtent;

		/*if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}*/
	}

}
