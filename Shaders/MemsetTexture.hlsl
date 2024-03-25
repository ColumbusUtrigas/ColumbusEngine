RWTexture2D<float4> Tex;

[[vk::push_constant]]
struct _Params {
	float4 Value;
    uint2  TextureSize;
} Params;

[numthreads(8,8,1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    if (any(Params.TextureSize.xy >= dispatchThreadId.xy))
        return;

    Tex[dispatchThreadId.xy] = Params.Value;
}