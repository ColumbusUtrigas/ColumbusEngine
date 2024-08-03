#pragma once

#include "Core/Assert.h"
#include "TextureVulkan.h"
#include "Common.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Columbus
{

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class SwapchainVulkan
	{
	public:
		VkDevice Device;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<Texture2*> Textures;

		uint32_t minImageCount;
		uint32_t imageCount;

		bool IsOutdated = false;

	public:
		SwapchainVulkan(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, SwapchainVulkan* OldSwapchain);
		~SwapchainVulkan();

	private:
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void CreateImageViews(VkDevice device);
		void DestroyImageViews();
	};

}