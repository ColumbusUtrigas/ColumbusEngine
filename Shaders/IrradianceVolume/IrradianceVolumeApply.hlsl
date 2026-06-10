#define SET 0
#include "../RayTracingIrradianceVolumes.hlsli"

[[vk::binding(0, 1)]]   Texture2D<float3> GBufferWP;
[[vk::binding(1, 1)]]   Texture2D<float3> GBufferNormal;
[[vk::binding(2, 1)]] RWTexture2D<half4> Output;

[[vk::push_constant]]
struct _Params
{
    uint2 Resolution;
    uint2 Padding;
} Params;

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    if (any(dtid.xy >= Params.Resolution))
        return;
    
    float3 WP = GBufferWP[dtid.xy];
    float3 Normal = GBufferNormal[dtid.xy];
    float3 Irradiance = SampleRuntimeIrradianceVolumes(WP, Normal);
    Output[dtid.xy] = float4(Irradiance, 1);
}
