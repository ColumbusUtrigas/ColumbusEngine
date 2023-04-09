#pragma once

#include <Graphics/Types.h>
#include <Core/SmartPointer.h>

namespace Columbus
{

    struct RayTracingPipelineDesc
    {
        SPtr<ShaderStage> RayGen;
        SPtr<ShaderStage> Miss;
        SPtr<ShaderStage> ClosestHit;

        uint32_t MaxRecursionDepth;
    };

    class RayTracingPipeline
    {
    private:
        RayTracingPipelineDesc _Desc;
    public:
        RayTracingPipeline(const RayTracingPipelineDesc& Desc) : _Desc(Desc) {}
		const RayTracingPipelineDesc& GetDesc() const { return _Desc; }
    };

}
