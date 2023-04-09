#pragma once

#include "Common.h"
#include <vulkan/vulkan.h>

namespace Columbus
{

    struct FenceVulkan
    {
    public:
        VkDevice _Device;
        VkFence _Fence;

    public:
        FenceVulkan(VkDevice Device, bool signaled) : _Device(Device)
        {
            VkFenceCreateInfo info;
            info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            info.pNext = nullptr;
            info.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

            VK_CHECK(vkCreateFence(Device, &info, nullptr, &_Fence));
        }

        ~FenceVulkan()
        {
            vkDestroyFence(_Device, _Fence, nullptr);
        }
    };

}
