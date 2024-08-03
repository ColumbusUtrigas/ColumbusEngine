[[vk::binding(0, 0)]]   Texture2D<float4> InRadiance;
[[vk::binding(1, 0)]]   Texture2D<float4> InRays;
[[vk::binding(2, 0)]]   Texture2D<float4> GBufferAlbedo;
[[vk::binding(3, 0)]]   Texture2D<float4> GBufferWP;
[[vk::binding(4, 0)]]   Texture2D<float4> GBufferNormal;
[[vk::binding(5, 0)]]   Texture2D<float2> GBufferRM;
[[vk::binding(6, 0)]]   Texture2D<float>  GBufferDepth;
[[vk::binding(7, 0)]] RWTexture2D<float4> Output;

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{

}