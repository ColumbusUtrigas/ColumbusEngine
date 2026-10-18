[[vk::binding(0, 0)]] Texture2D Source;
[[vk::binding(1, 0)]] SamplerState SourceSampler;
[[vk::binding(2, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;

struct RefractionBlurParameters
{
    int2 OutputSize;
};

[[vk::binding(3, 0)]] ConstantBuffer<RefractionBlurParameters> Params;

// Fixed, normalized 1D Gaussian kernels. Sigma is half the radius, matching the original 2D blur.
#if BLUR_KERNEL == 1
#define BLUR_RADIUS 2
static const float Weights[BLUR_RADIUS + 1] = { 0.40261994689, 0.24420134200, 0.05448868455 };
#elif BLUR_KERNEL == 2
#define BLUR_RADIUS 4
static const float Weights[BLUR_RADIUS + 1] = { 0.20416368872, 0.18017382291, 0.12383153681, 0.06628224529, 0.02763055064 };
#elif BLUR_KERNEL == 3
#define BLUR_RADIUS 8
static const float Weights[BLUR_RADIUS + 1] = { 0.10315261887, 0.09997894644, 0.09103186665, 0.07786368177, 0.06256522598, 0.04722671027, 0.03348875223, 0.02230831833, 0.01396018889 };
#endif

#if BLUR_KERNEL != 0
static const uint TileWidth  = 8;
static const uint TileHeight = 32;
static const uint CacheRows  = TileHeight + 2 * BLUR_RADIUS;

groupshared float4 HorizontalCache[TileWidth * CacheRows];

float4 FilterHorizontal(float2 UV, float2 InvSize)
{
    float4 Sum = Source.SampleLevel(SourceSampler, UV, 0) * Weights[0];

    [unroll]
    for (int i = 1; i <= BLUR_RADIUS; ++i)
    {
        float2 Offset = float2(i * InvSize.x, 0);
        Sum += (Source.SampleLevel(SourceSampler, UV - Offset, 0) + Source.SampleLevel(SourceSampler, UV + Offset, 0)) * Weights[i];
    }

    return Sum;
}
#endif

[numthreads(8, 8, 1)]
void main(uint3 DispatchThreadId : SV_DispatchThreadID, uint3 GroupId : SV_GroupID, uint3 GroupThreadId : SV_GroupThreadID, uint GroupIndex : SV_GroupIndex)
{
    float2 InvSize = rcp(float2(Params.OutputSize));

#if BLUR_KERNEL == 0
    if (any(DispatchThreadId.xy >= (uint2)Params.OutputSize))
    {
        return;
    }

    float2 UV = (float2(DispatchThreadId.xy) + 0.5) * InvSize;
    Output[DispatchThreadId.xy] = Source.SampleLevel(SourceSampler, UV, 0);
#else
    int2 TileOrigin = int2(GroupId.xy) * int2(TileWidth, TileHeight);

    // Cache the horizontal blur and its vertical halo. Keep halo UVs outside the output:
    // the source sampler must clamp them, preserving the downsampling footprint at image borders.
    for (uint i = GroupIndex; i < TileWidth * CacheRows; i += 64)
    {
        int2 Pixel = TileOrigin + int2(i % TileWidth, int(i / TileWidth) - BLUR_RADIUS);
        float2 UV  = (float2(Pixel) + 0.5) * InvSize;
        HorizontalCache[i] = FilterHorizontal(UV, InvSize);
    }

    // Every thread participates, including groups that partially overlap the output bounds.
    GroupMemoryBarrierWithGroupSync();

    [unroll]
    for (uint y = GroupThreadId.y; y < TileHeight; y += 8)
    {
        uint2 Pixel = uint2(TileOrigin) + uint2(GroupThreadId.x, y);

        if (any(Pixel >= (uint2)Params.OutputSize))
        {
            continue;
        }

        uint Center = (y + BLUR_RADIUS) * TileWidth + GroupThreadId.x;
        float4 Sum  = HorizontalCache[Center] * Weights[0];

        [unroll]
        for (uint i = 1; i <= BLUR_RADIUS; ++i)
        {
            Sum += (HorizontalCache[Center - i * TileWidth] + HorizontalCache[Center + i * TileWidth]) * Weights[i];
        }

        Output[Pixel] = Sum;
    }
#endif
}
