#pragma once

#include <Core/Assert.h>
#include <vulkan/vulkan.h>

#define VULKAN_DEBUG 1

bool IsVulkanDebugEnabled();

const char* VkResultToString(VkResult result);

#define VK_CHECK(x) { VkResult __vk_result = x; COLUMBUS_ASSERT_MESSAGE(__vk_result == VK_SUCCESS, VkResultToString(__vk_result)); }
