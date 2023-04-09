#pragma once

#include <vulkan/vulkan.h>

namespace Columbus
{

    // Dynamically loaded functions for extensions
    struct VulkanFunctions
    {
        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelines;
        PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructures;
        PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizes;
        PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructure;
        PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddress;
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandles;
        PFN_vkCmdTraceRaysKHR vkCmdTraceRays;

        void LoadFunctions(VkInstance instance, PFN_vkGetInstanceProcAddr vkInstanceGetProcAddr)
        {
            vkCreateRayTracingPipelines = (PFN_vkCreateRayTracingPipelinesKHR)vkInstanceGetProcAddr(instance, "vkCreateRayTracingPipelinesKHR");
            vkCmdBuildAccelerationStructures = (PFN_vkCmdBuildAccelerationStructuresKHR)vkInstanceGetProcAddr(instance, "vkCmdBuildAccelerationStructuresKHR");
            vkGetAccelerationStructureBuildSizes = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkInstanceGetProcAddr(instance, "vkGetAccelerationStructureBuildSizesKHR");
            vkCreateAccelerationStructure = (PFN_vkCreateAccelerationStructureKHR)vkInstanceGetProcAddr(instance, "vkCreateAccelerationStructureKHR");
            vkGetAccelerationStructureDeviceAddress = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkInstanceGetProcAddr(instance, "vkGetAccelerationStructureDeviceAddressKHR");
            vkGetRayTracingShaderGroupHandles = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkInstanceGetProcAddr(instance, "vkGetRayTracingShaderGroupHandlesKHR");
            vkCmdTraceRays = (PFN_vkCmdTraceRaysKHR)vkInstanceGetProcAddr(instance, "vkCmdTraceRaysKHR");
        }
    };

}
