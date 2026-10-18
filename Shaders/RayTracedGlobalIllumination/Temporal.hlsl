#include "Geometry.hlsli"
#include "Lighting.hlsli"

[[vk::binding(0, 0)]] Texture2D<float4> Input;
[[vk::binding(1, 0)]] Texture2D<float4> InputDirection;
[[vk::binding(2, 0)]] Texture2D<float4> HistorySH;
[[vk::binding(3, 0)]] Texture2D<float> Depth;
[[vk::binding(4, 0)]] Texture2D<float4> Normals;
[[vk::binding(5, 0)]] Texture2D<float4> SurfaceData;
[[vk::binding(6, 0)]] Texture2D<float> PreviousDepth;
[[vk::binding(7, 0)]] Texture2D<float4> PreviousNormals;
[[vk::binding(8, 0)]] Texture2D<float4> PreviousSurfaceData;
[[vk::binding(9, 0)]] Texture2D<float> SampleCountHistory;
[[vk::binding(10, 0)]] Texture2D<float4> Velocity;
[[vk::binding(11, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutputSH;
[[vk::binding(12, 0)]] [[vk::image_format("r16f")]] RWTexture2D<float> SampleCount;

// Suppress isolated bright rays before they enter either temporal history or
// spatial feedback. Only compatible current-frame surfaces establish the bound;
// accumulated history is never clamped to a noisy current neighbourhood.
float3 SuppressBrightSample(RTGISurface Surface, int2 Pixel, float3 Radiance)
{
    float Sum        = 0;
    float SumSquared = 0;
    int   Neighbours = 0;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            const int2 Candidate = Pixel + int2(x, y);

            if ((x == 0 && y == 0) || !RTGIValidTile(Candidate))
            {
                continue;
            }

            if (IsSkyDepth(Depth[Candidate]))
            {
                continue;
            }

            RTGISurface Other = RTGIGetSample(SurfaceData[Candidate], Normals[Candidate]);

            if (RTGISurfaceWeight(Surface, Other) <= 0.01)
            {
                continue;
            }

            const float Value = RTGILuminance(RTGIFiniteRadiance(Input[Candidate].rgb));
            Sum        += Value;
            SumSquared += Value * Value;
            ++Neighbours;
        }
    }

    const float Mean       = Sum / max(Neighbours, 1);
    const float Deviation  = sqrt(max(0, SumSquared / max(Neighbours, 1) - Mean * Mean));
    // Sparse hits on an ordinary emissive surface are legitimate lighting.
    // Reserve clipping for strong outliers; spatial repair handles their noise.
    const float UpperBound = max(4.0, Mean + 2.5 * Deviation + 0.05);
    const float Luminance  = RTGILuminance(Radiance);

    if (Neighbours >= 4 && Luminance > UpperBound)
    {
        Radiance *= UpperBound / Luminance;
    }

    return Radiance;
}

bool ReprojectProbe(RTGISurface Surface, float CurrentDepth, out RTGIProbeSH Result, out float Count)
{
    Result = (RTGIProbeSH)0;
    Count = 0;

    if (Geometry.Config.z == 0)
    {
        return false;
    }

    RTGISurface Expected;

    if (!RTGIReprojectSurface(Surface, CurrentDepth, Velocity[int2(Surface.Pixel)], Expected))
    {
        return false;
    }

    if (any(Expected.Pixel < -Geometry.Config.x) || any(Expected.Pixel >= Geometry.Sizes.xy + Geometry.Config.x))
    {
        return false;
    }

    const int2 Base = int2(floor(Expected.Pixel / Geometry.Config.x));
    float WeightSum = 0;

    for (int y = -1; y <= 1; ++y)
    {
        for (int x = -1; x <= 1; ++x)
        {
            const int2 Candidate = Base + int2(x, y);

            if (!RTGIValidTile(Candidate))
            {
                continue;
            }

            if (IsSkyDepth(PreviousDepth[Candidate]))
            {
                continue;
            }

            RTGISurface Other = RTGIGetSample(PreviousSurfaceData[Candidate], PreviousNormals[Candidate]);
            const float2 Tent = saturate(1 - abs(Other.Pixel - Expected.Pixel) / Geometry.Config.x);
            const float Weight = RTGISurfaceWeight(Expected, Other) * Tent.x * Tent.y;

            if (Weight > 1e-6)
            {
                RTGIAddProbe(Result, RTGIReadProbe(HistorySH, Candidate, Geometry.Sizes.w), Weight);
                Count += SampleCountHistory[Candidate] * Weight;
                WeightSum += Weight;
            }
        }
    }

    if (WeightSum <= 1e-6)
    {
        return false;
    }

    Result = RTGIScaleProbe(Result, 1 / WeightSum);
    Count /= WeightSum;
    return true;
}

[numthreads(8, 8, 1)]
void main(int2 Pixel : SV_DispatchThreadID)
{
    if (!RTGIValidTile(Pixel))
    {
        return;
    }

    if (IsSkyDepth(Depth[Pixel]))
    {
        RTGIWriteProbe(OutputSH, Pixel, Geometry.Sizes.w, (RTGIProbeSH)0);
        SampleCount[Pixel] = 0;
        return;
    }

    RTGISurface Surface = RTGIGetSample(SurfaceData[Pixel], Normals[Pixel]);
    RTGIProbeSH History;
    float HistoryCount;
    const bool HasHistory = ReprojectProbe(Surface, Depth[Pixel], History, HistoryCount);
    const float Count = HasHistory ? min(HistoryCount + 1, 64) : 1;
    const float4 Ray = Input[Pixel];
    float3 Radiance = RTGIFiniteRadiance(Ray.rgb);

    // Bootstrap has too few samples to represent rare bright directions safely.
    // Fade suppression out with real history age; mature probes retain the full
    // incoming signal instead of permanently biasing small emissive sources dark.
    if (Count < 16)
    {
        const float Strength = saturate((16 - Count) / 15);
        Radiance = lerp(Radiance, SuppressBrightSample(Surface, Pixel, Radiance), Strength);
    }

    RTGIProbeSH Current = RTGIProjectProbe(Radiance, InputDirection[Pixel].xyz, Ray.a < 0 ? 5000 : Ray.a);
    RTGIProbeSH Result = RTGIScaleProbe(Current, 1 / Count);
    RTGIAddProbe(Result, History, 1 - 1 / Count);

    RTGIWriteProbe(OutputSH, Pixel, Geometry.Sizes.w, Result);
    SampleCount[Pixel] = Count;
}
