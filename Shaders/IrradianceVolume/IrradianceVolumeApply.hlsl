#include "IrradianceVolumeCommon.hlsli"

[[vk::binding(0, 1)]]   Texture2D<float3> GBufferWP;
[[vk::binding(1, 1)]]   Texture2D<float3> GBufferNormal;
[[vk::binding(2, 1)]] RWTexture2D<float4> Output;

[[vk::push_constant]]
struct _Params
{
    // TODO: move to appropriate place
    float4 Position;
    float4 Extent;
    int4 ProbesCount;
    int4 ProbeIndex;
    
    uint2 Resolution;
} Params;

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    if (any(dtid.xy >= Params.Resolution))
        return;
    
    float3 WP = GBufferWP[dtid.xy];
    float3 Normal = GBufferNormal[dtid.xy];
    
    IrradianceVolume Volume;
    Volume.Position = Params.Position.xyz;
    Volume.Extent = Params.Extent.xyz;
    Volume.ProbesCount = Params.ProbesCount.xyz;
    
    Output[dtid.xy] = float4(SampleIrradianceProbes(Volume, WP, Normal), 1);
}