[[vk::binding(0, 0)]]   Texture2D<float4> InRadiance;
[[vk::binding(1, 0)]]   Texture2D<float4> InRays;
[[vk::binding(2, 0)]]   Texture2D<float>  InRayPdf;
[[vk::binding(3, 0)]]   Texture2D<float4> GBufferAlbedo;
[[vk::binding(4, 0)]]   Texture2D<float4> GBufferWP;
[[vk::binding(5, 0)]]   Texture2D<float4> GBufferNormal;
[[vk::binding(6, 0)]]   Texture2D<float2> GBufferRM;
[[vk::binding(7, 0)]]   Texture2D<float>  GBufferDepth;
[[vk::binding(8, 0)]] [[vk::image_format("rgba16f")]] RWTexture2D<float4> Output;
[[vk::binding(9, 0)]] [[vk::image_format("r16f")]] RWTexture2D<float> OutputHitDistance;

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

float RadicalInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

float2 Hammersley(uint i, uint n, uint seed)
{
    return float2((float(i) + Random::FloatConstruct(seed)) / float(n), RadicalInverseVdC(i ^ seed));
}

float2 SampleDisk(float2 u)
{
    float r = sqrt(u.x);
    float phi = TWO_PI * u.y;
    return r * float2(cos(phi), sin(phi));
}

float GetSampleWeight(float3 rayDir, float rayPdf, BRDFData brdf)
{
    brdf.L = rayDir;
    float3 brdfValue = max(EvaluateReflectionBRDF(brdf), 0.0);
    return max(Luminance(brdfValue), 1e-5) / max(rayPdf, 1e-5);
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
        OutputHitDistance[pixel] = -1.0;
        return;
    }

    float4 centerRayPacked = InRays[pixel];
    float centerRayPdf = InRayPdf[pixel];
    if (IsInvalidRay(centerRayPacked, centerRayPdf))
    {
        Output[pixel] = centerRadiance;
        OutputHitDistance[pixel] = -1.0;
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

    const float minNormalDot = lerp(0.9995, 0.9000, centerRoughness);
    const float maxPlaneDistance = lerp(0.0010, 0.0800, centerRoughness);
    const float maxRoughnessDiff = lerp(0.0200, 0.2500, centerRoughness);
    const float maxReuseRatio = lerp(4.0, 24.0, centerRoughness);
    const float kernelRadius = centerRoughness > 0.05 ? max(1.0, 18.0 * saturate(centerRoughness * 2.0)) : 0.0;
    const uint sampleCount = centerRoughness > 0.05 ? 24 : 0;

    float centerSampleWeight = GetSampleWeight(centerRayDir, centerRayPdf, centerBrdfBase);
    centerSampleWeight = max(centerSampleWeight, 0.001);

    float3 accumulated = centerRadiance.rgb * centerSampleWeight;
    float weightSum = centerSampleWeight;
    float minHitDistance = centerHasHit ? centerRayPacked.w : -1.0;

    uint seed = Random::Hash(uint(pixel.x) ^ Random::Hash(uint(pixel.y)));

    [loop]
    for (uint i = 0; i < sampleCount; ++i)
    {
        float2 disk = SampleDisk(Hammersley(i, sampleCount, seed));
        int2 samplePixel = pixel + int2(round(disk * kernelRadius));
        if (all(samplePixel == pixel))
            continue;

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
        float3 directionToSampleHit = sampleRayDir;
        float sampleHitDistance = sampleRayPacked.w;

        if (centerHasHit)
        {
            sampleHitDistance = min(sampleHitDistance, centerRayPacked.w);
        }

        if (sampleHasHit)
        {
            float3 sampleHitPosition = sampleWP + sampleRayDir * sampleHitDistance;
            float3 centerToSampleHit = sampleHitPosition - centerWP;
            float sampleDistanceFromCenter = length(centerToSampleHit);
            if (sampleDistanceFromCenter > 1e-4)
            {
                directionToSampleHit = centerToSampleHit / sampleDistanceFromCenter;
                sampleHitDistance = sampleDistanceFromCenter;
            }
        }

        float normalWeight = saturate((normalDot - minNormalDot) / max(1.0 - minNormalDot, 1e-4));
        float roughnessWeight = 1.0 - saturate(roughnessDiff / max(maxRoughnessDiff, 1e-4));
        float planeWeight = 1.0 - saturate(planeDistance / max(maxPlaneDistance, 1e-4));
        float geometryWeight = normalWeight * roughnessWeight * planeWeight;
        if (geometryWeight <= 1e-4)
            continue;

        BRDFData centerBrdf = centerBrdfBase;
        float sampleWeight = GetSampleWeight(directionToSampleHit, sampleRayPdf, centerBrdf);
        sampleWeight = min(sampleWeight, centerSampleWeight * maxReuseRatio);

        float reuseWeight = geometryWeight * sampleWeight;
        accumulated += InRadiance[samplePixel].rgb * reuseWeight;
        weightSum += reuseWeight;

        if (sampleHasHit)
        {
            minHitDistance = minHitDistance > 0.0 ? min(minHitDistance, sampleHitDistance) : sampleHitDistance;
        }
    }

    float3 resolved = (weightSum > EPSILON) ? (accumulated / weightSum) : centerRadiance.rgb;
    Output[pixel] = float4(resolved, centerRadiance.a);
    OutputHitDistance[pixel] = minHitDistance;
}
