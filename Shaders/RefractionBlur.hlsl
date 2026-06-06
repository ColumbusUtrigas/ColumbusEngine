[[vk::binding(0, 0)]] Texture2D Source;
[[vk::binding(1, 0)]] SamplerState SourceSampler;
[[vk::binding(2, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;

[[vk::push_constant]]
struct _Params
{
    int2 OutputSize;
    int Radius;
    float _pad;
} Params;

float GaussianWeight(float2 Offset, float Sigma)
{
    return exp(-dot(Offset, Offset) / max(2.0 * Sigma * Sigma, 0.0001));
}

[numthreads(8, 8, 1)]
void main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
    if (DispatchThreadId.x >= (uint)Params.OutputSize.x || DispatchThreadId.y >= (uint)Params.OutputSize.y)
    {
        return;
    }

    float2 InvSize = rcp(float2(Params.OutputSize));
    float2 UV = (float2(DispatchThreadId.xy) + 0.5) * InvSize;

    if (Params.Radius <= 0)
    {
        Output[DispatchThreadId.xy] = Source.SampleLevel(SourceSampler, UV, 0);
        return;
    }

    float Sigma = max((float)Params.Radius * 0.5, 1.0);
    float4 Sum = 0;
    float WeightSum = 0;

    [loop]
    for (int y = -Params.Radius; y <= Params.Radius; y++)
    {
        [loop]
        for (int x = -Params.Radius; x <= Params.Radius; x++)
        {
            float2 Offset = float2(x, y);
            float Weight = GaussianWeight(Offset, Sigma);
            Sum += Source.SampleLevel(SourceSampler, UV + Offset * InvSize, 0) * Weight;
            WeightSum += Weight;
        }
    }

    Output[DispatchThreadId.xy] = Sum / max(WeightSum, 0.0001);
}
