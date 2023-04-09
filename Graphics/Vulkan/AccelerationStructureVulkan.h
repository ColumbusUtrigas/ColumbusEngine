#pragma once

#include <Math/Matrix.h>
#include "BufferVulkan.h"
#include <vulkan/vulkan.h>
#include <vector>

namespace Columbus
{

    enum class AccelerationStructureType
    {
        BLAS, // Bottom-level
        TLAS, // Top-level
    };

    class AccelerationStructureVulkan;

    struct AccelerationStructureInstance
    {
        Matrix Transform;
        AccelerationStructureVulkan* Blas;
    };

    struct AccelerationStructureDesc
    {
        AccelerationStructureType Type;

        // BLAS-only
        BufferVulkan Geometry;
        BufferVulkan Indices;
        int VerticesCount;
        int IndicesCount;
        bool Indexed;

        // TLAS-only
        std::vector<AccelerationStructureInstance> Instances;
    };

    class AccelerationStructureVulkan
    {
    public:
        AccelerationStructureDesc _Desc;
        VkAccelerationStructureKHR _Handle;
        BufferVulkan _Buffer;
        uint64_t _DeviceAddress;
    
    public:
        AccelerationStructureVulkan(const AccelerationStructureDesc& Desc) : _Desc(Desc) {}
    };

}
