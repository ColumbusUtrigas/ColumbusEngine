#include "Geometry.hlsli"
#include "Lighting.hlsli"

[[vk::binding(0, 0)]] Texture2D<float4> InputSH;
[[vk::binding(1, 0)]] Texture2D<float> Depth;
[[vk::binding(2, 0)]] Texture2D<float2> Normals;
[[vk::binding(3, 0)]] Texture2D<float> SampleDepth;
[[vk::binding(4, 0)]] Texture2D<float4> SampleNormals;
[[vk::binding(5, 0)]] Texture2D<float4> SurfaceData;
[[vk::binding(6, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;
[[vk::binding(7, 0)]] Texture2D<float4> PreviousRadiance;
[[vk::binding(8, 0)]] Texture2D<float> PreviousDepth;
[[vk::binding(9, 0)]] Texture2D<float2> PreviousNormals;
[[vk::binding(10, 0)]] Texture2D<float4> Velocity;
[[vk::binding(11, 0)]] Texture2D<float2> ShadingNormals;
[[vk::binding(12, 0)]] Texture2D<float4> RayInput;
[[vk::binding(13, 0)]] Texture2D<float4> RayDirection;
[[vk::binding(14, 0)]] Texture2D<float> SampleCount;

[[vk::push_constant]]
struct _Params
{
    uint HistoryValid;
    uint UseDenoiser;
} Params;

void GatherProbe(RTGISurface Surface, int2 Candidate, float BilinearWeight, float ViewDepth, inout RTGIProbeSH Result, inout float WeightSum)
{
    if (!RTGIValidTile(Candidate) || BilinearWeight <= 0)
    {
        return;
    }

    if (IsSkyDepth(SampleDepth[Candidate]))
    {
        return;
    }

    RTGISurface ProbeSurface = RTGIGetSample(SurfaceData[Candidate], SampleNormals[Candidate]);
    const float3 Delta = ProbeSurface.Position - Surface.Position;
    const float PlaneDistance = abs(dot(Delta, Surface.GeometricNormal));
    const float NormalWeight = saturate(1 + dot(Surface.GeometricNormal, ProbeSurface.GeometricNormal));
    float Weight = BilinearWeight * NormalWeight * exp2(-1000 * PlaneDistance / max(ViewDepth, 0.01));

    if (Weight <= 1e-6)
    {
        return;
    }

    RTGIProbeSH Probe;

    if (Params.UseDenoiser != 0)
    {
        Probe = RTGIReadProbe(InputSH, Candidate, Geometry.Sizes.w);
        Weight *= RTGIProbeConfidence(SampleCount[Candidate]);
    }
    else
    {
        const float4 Ray = RayInput[Candidate];
        Probe = RTGIProjectProbe(Ray.rgb, RayDirection[Candidate].xyz, Ray.a < 0 ? 5000 : Ray.a);
    }

    RTGIAddProbe(Result, Probe, Weight);
    WeightSum += Weight;
}

// Stabilize lighting at the receiver, after changing low-resolution supports
// have been combined. Validate every bilinear history tap against full geometry.
float4 StabilizeReceiver(RTGISurface Surface, float3 Current, bool Supported)
{
    float3 HistoryValue = 0;
    float  HistoryAge   = 0;
    float  WeightSum    = 0;
    const int2 Pixel = int2(Surface.Pixel);
    RTGISurface Expected;
    const bool Reprojected = RTGIReprojectSurface(Surface, Depth[Pixel], Velocity[Pixel], Expected) && Params.HistoryValid != 0;
    const float2 PreviousPixel = Expected.Pixel;

    if (Reprojected && all(PreviousPixel > -1) && all(PreviousPixel < Geometry.Sizes.xy))
    {
        const int2 Base = int2(floor(PreviousPixel));
        const float2 Fraction = frac(PreviousPixel);

        [unroll]
        for (int y = 0; y <= 1; ++y)
        {
            [unroll]
            for (int x = 0; x <= 1; ++x)
            {
                const int2 Candidate = Base + int2(x, y);
                const float2 Tent = lerp(1 - Fraction, Fraction, float2(x, y));
                const float PixelWeight = Tent.x * Tent.y;

                if (PixelWeight <= 1e-6 || any(Candidate < 0) || any(Candidate >= Geometry.Sizes.xy))
                {
                    continue;
                }

                const float4 Previous = PreviousRadiance[Candidate];
                const float DepthValue = PreviousDepth[Candidate];

                if (Previous.a < 1 || IsSkyDepth(DepthValue))
                {
                    continue;
                }

                RTGISurface Other = RTGIGetSurface(Candidate, DepthValue, PreviousNormals[Candidate], true);
                const float Weight = RTGISurfaceWeight(Expected, Other) * PixelWeight;
                HistoryValue += Previous.rgb * Weight;
                HistoryAge   += Previous.a * Weight;
                WeightSum    += Weight;
            }
        }
    }

    if (WeightSum <= 1e-6)
    {
        return Supported ? float4(Current, 1) : 0;
    }

    HistoryValue /= WeightSum;
    HistoryAge   /= WeightSum;

    if (!Supported)
    {
        // Bridge a temporary coverage hole with proven same-surface lighting,
        // but expire it instead of carrying unsupported lighting indefinitely.
        const float Age = max(min(HistoryAge, 8) - 1, 0);
        return Age >= 1 ? float4(RTGIFiniteRadiance(HistoryValue), Age) : 0;
    }

    // Cover the ten-frame projection-jitter cycle rather than allowing
    // changing coarse supports to dominate the final receiver each cycle.
    const float Age = min(HistoryAge + 1, 16);
    return float4(RTGIFiniteRadiance(lerp(HistoryValue, Current, 1 / Age)), Age);
}

[numthreads(8, 8, 1)]
void main(int2 Pixel : SV_DispatchThreadID)
{
    if (any(Pixel >= Geometry.Sizes.xy))
    {
        return;
    }

    if (IsSkyDepth(Depth[Pixel]))
    {
        Output[Pixel] = 0;
        return;
    }

    RTGISurface Surface = RTGIGetSurface(Pixel, Depth[Pixel], Normals[Pixel]);
    const float ViewDepth = Geometry.DepthProjection.y / (Depth[Pixel] + Geometry.DepthProjection.x);
    const float2 ProbePixel = (float2(Pixel) + 0.5) / Geometry.Config.x - 0.5;
    const int2 Base = int2(floor(ProbePixel));
    // Expand the four-corner footprint slightly to avoid near-zero support
    // exactly on a probe while adjacent probes represent the receiver better.
    const float2 Fraction = (frac(ProbePixel) * Geometry.Config.x + 1) / (Geometry.Config.x + 2);
    RTGIProbeSH Result = (RTGIProbeSH)0;
    float WeightSum = 0;

    [unroll]
    for (int y = 0; y <= 1; ++y)
    {
        [unroll]
        for (int x = 0; x <= 1; ++x)
        {
            const float2 Tent = lerp(1 - Fraction, Fraction, float2(x, y));
            GatherProbe(Surface, Base + int2(x, y), Tent.x * Tent.y, ViewDepth, Result, WeightSum);
        }
    }

    if (WeightSum <= 1e-8)
    {
        const int2 Tile = Pixel / Geometry.Config.x;

        for (int y = -2; y <= 2; ++y)
        {
            for (int x = -2; x <= 2; ++x)
            {
                GatherProbe(Surface, Tile + int2(x, y), 1, ViewDepth, Result, WeightSum);
            }
        }
    }

    const bool Supported = WeightSum > 1e-8;
    float3 Current = 0;

    if (Supported)
    {
        const float3 LightingNormal = NormalDecode(ShadingNormals[Pixel]);
        Current = RTGIFiniteRadiance(RTGIEvaluateProbe(RTGIScaleProbe(Result, 1 / WeightSum), LightingNormal));
    }

    Output[Pixel] = StabilizeReceiver(Surface, Current, Supported);
}
