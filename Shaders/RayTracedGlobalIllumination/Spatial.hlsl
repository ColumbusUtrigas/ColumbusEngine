#include "Geometry.hlsli"
#include "Lighting.hlsli"

[[vk::binding(0, 0)]] Texture2D<float4> InputSH;
[[vk::binding(1, 0)]] Texture2D<float> Depth;
[[vk::binding(2, 0)]] Texture2D<float4> Normals;
[[vk::binding(3, 0)]] Texture2D<float4> SurfaceData;
[[vk::binding(4, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> OutputSH;
[[vk::binding(5, 0)]] Texture2D<float> SampleCount;

[[vk::push_constant]]
struct _Params
{
    int StepSize;
} Params;

void GatherNeighbour(int2 Candidate, RTGISurface Surface, RTGIProbeSH Center, float CenterCount, float ViewDepth, float KernelWeight, inout RTGIProbeSH Result, inout float WeightSum)
{
    if (!RTGIValidTile(Candidate))
    {
        return;
    }

    if (IsSkyDepth(Depth[Candidate]))
    {
        return;
    }

    RTGISurface Other = RTGIGetSample(SurfaceData[Candidate], Normals[Candidate]);
    const float3 Delta = Other.Position - Surface.Position;
    const float NormalWeight = saturate(dot(Surface.GeometricNormal, Other.GeometricNormal));
    const float PlaneDistance = max(abs(dot(Delta, Surface.GeometricNormal)), abs(dot(Delta, Other.GeometricNormal)));
    const float PlaneWeight = exp2(-1000 * PlaneDistance / max(ViewDepth, 0.01));
    float Weight = NormalWeight * PlaneWeight * KernelWeight;

    if (Weight < 1e-4)
    {
        return;
    }

    const float OtherCount = SampleCount[Candidate];
    Weight *= RTGIProbeConfidence(OtherCount);
    RTGIProbeSH Sample = RTGIReadProbe(InputSH, Candidate, Geometry.Sizes.w);

    // A single short ray is not a reliable visibility bound for an entire probe.
    // Fade the existing mean-distance restriction in only after both estimates
    // have accumulated support. Surface/normal rejection remains active throughout.
    const float HitConfidence = saturate((min(CenterCount, OtherCount) - 4) / 12);
    const float HitDistance = max(min(Center.Last.w, Sample.Last.w), Surface.PixelSize * Geometry.Config.x);
    const float HitWeight = saturate(1 - length(Delta) / max(HitDistance, 0.001));
    Weight *= lerp(1.0, HitWeight, HitConfidence);

    RTGIAddProbe(Result, Sample, Weight);
    WeightSum += Weight;
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
        return;
    }

    RTGISurface Surface = RTGIGetSample(SurfaceData[Pixel], Normals[Pixel]);
    RTGIProbeSH Center = RTGIReadProbe(InputSH, Pixel, Geometry.Sizes.w);
    const float Count      = SampleCount[Pixel];
    const float Confidence = RTGIProbeConfidence(Count);
    const float Expansion  = 1 - saturate((Count - 1) / 31);

    RTGIProbeSH Result = RTGIScaleProbe(Center, Confidence);
    float WeightSum = Confidence;
    const float ViewDepth = Geometry.DepthProjection.y / (Depth[Pixel] + Geometry.DepthProjection.x);

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            if (x == 0 && y == 0)
            {
                continue;
            }

            const float NarrowKernel = (x == 0 || y == 0) ? 0.5 : 0.25;
            const float2 Tent = 1 - abs(float2(x, y)) / 3;
            const float Kernel = lerp(NarrowKernel, Tent.x * Tent.y, Expansion);

            GatherNeighbour(Pixel + int2(x, y) * Params.StepSize, Surface, Center, Count, ViewDepth, Kernel, Result, WeightSum);
        }
    }

    // Fresh receivers prefer blur to Monte Carlo patches. Use a dense wider
    // final kernel instead of another sparse a-trous pass, without another image.
    // Only the first, narrow pass is retained for the next frame.
    if (Expansion > 0)
    {
        const int Radius = Params.StepSize == 4 ? 6 : 2;

        for (int y = -Radius; y <= Radius; ++y)
        {
            for (int x = -Radius; x <= Radius; ++x)
            {
                if (abs(x) <= 1 && abs(y) <= 1)
                {
                    continue;
                }

                const float2 Tent = 1 - abs(float2(x, y)) / float(Radius + 1);
                const float Kernel = Tent.x * Tent.y * Expansion;
                GatherNeighbour(Pixel + int2(x, y) * Params.StepSize, Surface, Center, Count, ViewDepth, Kernel, Result, WeightSum);
            }
        }
    }

    // The distance is a visibility guide for this probe, not borrowed lighting.
    Result = RTGIScaleProbe(Result, 1 / WeightSum);
    Result.Last.w = Center.Last.w;
    RTGIWriteProbe(OutputSH, Pixel, Geometry.Sizes.w, Result);
}
