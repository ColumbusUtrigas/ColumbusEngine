[[vk::binding(0, 0)]]   Texture2D<float4> InRadiance;
[[vk::binding(1, 0)]]   Texture2D<float4> InRays;
[[vk::binding(2, 0)]]   Texture2D<float>  InRayPdf;
[[vk::binding(3, 0)]]   Texture2D<float4> GBufferAlbedo;
[[vk::binding(4, 0)]]   Texture2D<float4> GBufferWP;
[[vk::binding(5, 0)]]   Texture2D<float4> GBufferNormal;
[[vk::binding(6, 0)]]   Texture2D<float2> GBufferRM;
[[vk::binding(7, 0)]]   Texture2D<float>  GBufferDepth;
[[vk::binding(8, 0)]] RWTexture2D<float4> Output;

#include "../Common.hlsli"
#include "../BRDF.hlsli"

[[vk::push_constant]]
struct _Params
{
    float4 CameraPosition;
    int2 ImageSize;
    int2 _Padding;
} Params;

bool IsSkyDepth(float depth)
{
    return abs(depth) < EPSILON || abs(depth - 1.0) < EPSILON;
}

bool IsInvalidRay(float4 rayDirectionDistance, float rayPdf)
{
    return rayPdf <= 0.0 || (rayDirectionDistance.w < 0.0 && dot(rayDirectionDistance.xyz, rayDirectionDistance.xyz) < 1e-6);
}

float Luminance(float3 x)
{
    return dot(x, float3(0.299, 0.587, 0.114));
}

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    int2 pixel = int2(dtid.xy);
    if (any(pixel >= Params.ImageSize))
        return;

    float4 centerRadiance = InRadiance[pixel];
    float centerDepth = GBufferDepth[pixel];
    if (IsSkyDepth(centerDepth))
    {
        Output[pixel] = centerRadiance;
        return;
    }

    float4 centerRayPacked = InRays[pixel];
    float centerRayPdf = InRayPdf[pixel];
    if (IsInvalidRay(centerRayPacked, centerRayPdf))
    {
        Output[pixel] = centerRadiance;
        return;
    }

    float2 centerRM = GBufferRM[pixel].xy;
    float centerRoughness = saturate(centerRM.x);
    float centerMetallic = saturate(centerRM.y);

    float3 centerNormal = normalize(GBufferNormal[pixel].xyz);
    float3 centerAlbedo = GBufferAlbedo[pixel].rgb;
    float3 centerWP = GBufferWP[pixel].xyz;
    float3 centerV = normalize(Params.CameraPosition.xyz - centerWP);
    float3 centerRayDir = normalize(centerRayPacked.xyz);
    bool centerHasHit = centerRayPacked.w > 0.0;

    BRDFData centerBrdfBase;
    centerBrdfBase.N = centerNormal;
    centerBrdfBase.V = centerV;
    centerBrdfBase.Albedo = centerAlbedo;
    centerBrdfBase.Roughness = max(centerRoughness, 0.001);
    centerBrdfBase.Metallic = centerMetallic;

    const float minNormalDot = lerp(0.9995, 0.9500, centerRoughness);
    const float maxPlaneDistance = lerp(0.0010, 0.0300, centerRoughness);
    const float maxRoughnessDiff = lerp(0.0200, 0.2200, centerRoughness);
    const float minRayDirDot = lerp(0.9995, 0.3500, centerRoughness);
    const float maxHitDistanceRatio = lerp(0.0500, 0.5000, centerRoughness);
    const float maxReuseRatio = lerp(4.0, 20.0, centerRoughness);

    static const int2 kOffsets[12] = {
        int2(-2, 0), int2(-1, -1), int2(-1, 0), int2(-1, 1),
        int2(0, -2), int2(0, -1),               int2(0, 1), int2(0, 2),
        int2(1, -1), int2(1, 0), int2(1, 1),
        int2(2, 0)
    };

    float3 accumulated = centerRadiance.rgb;
    float weightSum = 1.0;

    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        int2 samplePixel = pixel + kOffsets[i];
        if (any(samplePixel < 0) || any(samplePixel >= Params.ImageSize))
            continue;

        float sampleDepth = GBufferDepth[samplePixel];
        if (IsSkyDepth(sampleDepth))
            continue;

        float4 sampleRayPacked = InRays[samplePixel];
        float sampleRayPdf = InRayPdf[samplePixel];
        if (IsInvalidRay(sampleRayPacked, sampleRayPdf))
            continue;

        bool sampleHasHit = sampleRayPacked.w > 0.0;
        if (sampleHasHit != centerHasHit)
            continue;

        float2 sampleRM = GBufferRM[samplePixel].xy;
        float sampleRoughness = saturate(sampleRM.x);
        float sampleMetallic = saturate(sampleRM.y);

        float roughnessDiff = abs(sampleRoughness - centerRoughness);
        if (roughnessDiff > maxRoughnessDiff)
            continue;

        float3 sampleNormal = normalize(GBufferNormal[samplePixel].xyz);
        float normalDot = saturate(dot(centerNormal, sampleNormal));
        if (normalDot < minNormalDot)
            continue;

        float3 sampleWP = GBufferWP[samplePixel].xyz;
        float planeDistance = abs(dot(sampleWP - centerWP, centerNormal));
        if (planeDistance > maxPlaneDistance)
            continue;

        float3 sampleRayDir = normalize(sampleRayPacked.xyz);
        float rayDirDot = saturate(dot(centerRayDir, sampleRayDir));
        if (rayDirDot < minRayDirDot)
            continue;

        if (centerHasHit)
        {
            float hitDistanceRatio = abs(sampleRayPacked.w - centerRayPacked.w) / max(centerRayPacked.w, 1.0);
            if (hitDistanceRatio > maxHitDistanceRatio)
                continue;
        }

        float normalWeight = saturate((normalDot - minNormalDot) / max(1.0 - minNormalDot, 1e-4));
        float roughnessWeight = 1.0 - saturate(roughnessDiff / max(maxRoughnessDiff, 1e-4));
        float rayWeight = saturate((rayDirDot - minRayDirDot) / max(1.0 - minRayDirDot, 1e-4));
        float planeWeight = 1.0 - saturate(planeDistance / max(maxPlaneDistance, 1e-4));
        float sampleWeight = max(normalWeight * roughnessWeight * rayWeight * planeWeight, 1e-3);

        BRDFData sampleBrdf;
        sampleBrdf.N = sampleNormal;
        sampleBrdf.V = normalize(Params.CameraPosition.xyz - sampleWP);
        sampleBrdf.L = sampleRayDir;
        sampleBrdf.Albedo = GBufferAlbedo[samplePixel].rgb;
        sampleBrdf.Roughness = max(sampleRoughness, 0.001);
        sampleBrdf.Metallic = sampleMetallic;

        BRDFData centerBrdf = centerBrdfBase;
        centerBrdf.L = sampleRayDir;

        float centerFactor = max(Luminance(max(EvaluateReflectionBRDF(centerBrdf), 0.0)), 1e-5);
        float sampleFactor = max(Luminance(max(EvaluateReflectionBRDF(sampleBrdf), 0.0)), 1e-5);
        float samplePdfEval = max(BRDFCalcPDF(sampleBrdf), 1e-5);

        float ratio = centerFactor * sampleRayPdf / max(sampleFactor * samplePdfEval, 1e-5);
        ratio = clamp(ratio, 0.25, maxReuseRatio);

        float reuseWeight = sampleWeight * ratio;
        accumulated += InRadiance[samplePixel].rgb * reuseWeight;
        weightSum += reuseWeight;
    }

    float3 resolved = (weightSum > EPSILON) ? (accumulated / weightSum) : centerRadiance.rgb;
    Output[pixel] = float4(resolved, centerRadiance.a);
}
