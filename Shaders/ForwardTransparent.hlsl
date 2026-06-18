#pragma pack_matrix(row_major)

#include "BRDF.hlsli"
#include "GPUScene.hlsli"

[[vk::push_constant]]
struct _Params
{
    uint ObjectId;
} Parameters;

[[vk::binding(0, 2)]] Texture2D RefractionLevel0;
[[vk::binding(1, 2)]] Texture2D RefractionLevel1;
[[vk::binding(2, 2)]] Texture2D RefractionLevel2;
[[vk::binding(3, 2)]] Texture2D RefractionLevel3;
[[vk::binding(4, 2)]] SamplerState RefractionSampler;

struct VS_TO_PS
{
    float4 Pos      : SV_POSITION;
    float3 Normal   : NORMAL0;
    float3 Tangent  : NORMAL1;
    float3 Bitangent: NORMAL2;
    float2 Uv       : TEXCOORD0;
    float3 WorldPos : POSITION0;

    nointerpolation uint MaterialId : COLOR0;
};

VS_TO_PS VSMain(uint VertexId : SV_VertexID)
{
    VS_TO_PS Out;

    GPUSceneMeshCompact Mesh = GPUScene::GPUSceneMeshes[Parameters.ObjectId];
    GPUScene::Vertex Vertex = GPUScene::FetchVertex(Parameters.ObjectId, VertexId);

    float3x3 NormalMatrix = (float3x3) 0;
    NormalMatrix[0] = Mesh.NormalMatrix[0].xyz;
    NormalMatrix[1] = Mesh.NormalMatrix[1].xyz;
    NormalMatrix[2] = Mesh.NormalMatrix[2].xyz;

    Out.Normal = normalize(mul(NormalMatrix, Vertex.Normal));
    Out.Tangent = normalize(mul(NormalMatrix, Vertex.TangentAndSign.xyz));
    Out.Bitangent = cross(Out.Normal, Out.Tangent) * Vertex.TangentAndSign.w;

    float4 TransformedPos = mul(Mesh.Transform, float4(Vertex.Position, 1));
    Out.WorldPos = TransformedPos.xyz;
    Out.Pos = mul(GPUScene::GPUSceneScene[0].CameraCur.ViewProjectionMatrix, TransformedPos) * float4(1, -1, 1, 1);
    Out.Uv = Vertex.UV;
    Out.MaterialId = Mesh.MaterialId;

    return Out;
}

float3 EvaluateUnshadowedDirectLighting(BRDFData BRDF, float3 WorldPos)
{
    float3 LightingSum = 0;

    for (uint i = 0; i < GPUScene::GPUSceneScene[0].LightsCount; i++)
    {
        GPULight Light = GPUScene::GPUSceneLights[i];

        float3 LightDir = Light.Direction.xyz;
        float Distance = distance(WorldPos, Light.Position.xyz);
        float Attenuation = 0;
        float3 LightValue = 0;

        switch (Light.Type)
        {
        case GPULIGHT_DIRECTIONAL:
            BRDF.L = LightDir;
            LightingSum += EvaluateBRDFCos(BRDF) * Light.Color.rgb;
            break;

        case GPULIGHT_POINT:
            Attenuation = clamp(1.0 - Distance * Distance / (Light.Range * Light.Range), 0.0, 1.0);
            Attenuation *= Attenuation;
            BRDF.L = -normalize(WorldPos - Light.Position.xyz);
            LightValue = Attenuation * Light.Color.rgb;
            LightingSum += EvaluateBRDFCos(BRDF) * LightValue;
            break;

        case GPULIGHT_SPOT:
        {
            Attenuation = clamp(1.0 - Distance * Distance / (Light.Range * Light.Range), 0.0, 1.0);
            Attenuation *= Attenuation;
            BRDF.L = -normalize(WorldPos - Light.Position.xyz);

            float Angle = saturate(dot(BRDF.L, Light.Direction.xyz));
            Attenuation *= smoothstep(Light.SizeOrSpotAngles.y, Light.SizeOrSpotAngles.x, Angle);

            LightValue = Attenuation * Light.Color.rgb;
            LightingSum += EvaluateBRDFCos(BRDF) * LightValue;
        }
        break;

        case GPULIGHT_RECTANGLE:
        case GPULIGHT_DISC:
        case GPULIGHT_LINE:
            // Keep transparent forward lean for now; proper area LTC can be shared later.
            Attenuation = clamp(1.0 - Distance * Distance / (Light.Range * Light.Range), 0.0, 1.0);
            Attenuation *= Attenuation;
            BRDF.L = -normalize(WorldPos - Light.Position.xyz);
            LightValue = Attenuation * Light.Color.rgb;
            LightingSum += EvaluateBRDFCos(BRDF) * LightValue;
            break;

        default:
            break;
        }
    }

    return LightingSum;
}

float CubicWeight(float x)
{
    x = abs(x);

    if (x <= 1.0)
    {
        return ((1.5 * x - 2.5) * x) * x + 1.0;
    }

    if (x < 2.0)
    {
        return (((-0.5 * x + 2.5) * x - 4.0) * x) + 2.0;
    }

    return 0.0;
}

float3 SampleBicubic(Texture2D Texture, float2 UV)
{
    uint Width;
    uint Height;
    Texture.GetDimensions(Width, Height);

    float2 Size = float2(Width, Height);
    float2 InvSize = rcp(Size);
    float2 Pixel = saturate(UV) * Size - 0.5;
    float2 Base = floor(Pixel);
    float2 Frac = Pixel - Base;

    float4 Sum = 0;
    float WeightSum = 0;

    [unroll]
    for (int y = -1; y <= 2; y++)
    {
        float WeightY = CubicWeight((float)y - Frac.y);

        [unroll]
        for (int x = -1; x <= 2; x++)
        {
            float WeightX = CubicWeight((float)x - Frac.x);
            float Weight = WeightX * WeightY;
            float2 SampleUV = (Base + float2(x, y) + 0.5) * InvSize;
            SampleUV = clamp(SampleUV, 0.5 * InvSize, 1.0 - 0.5 * InvSize);
            Sum += Texture.SampleLevel(RefractionSampler, SampleUV, 0) * Weight;
            WeightSum += Weight;
        }
    }

    return Sum.rgb / max(WeightSum, 0.0001);
}

float3 SampleRefractionLevel(int Level, float2 UV)
{
    switch (Level)
    {
    case 0: return SampleBicubic(RefractionLevel0, UV);
    case 1: return SampleBicubic(RefractionLevel1, UV);
    case 2: return SampleBicubic(RefractionLevel2, UV);
    default: return SampleBicubic(RefractionLevel3, UV);
    }
}

float3 SampleRoughRefraction(float2 ScreenUV, float3 TangentNormal, float Roughness, float Alpha)
{
    float DistortionStrength = lerp(0.02, 0.10, 1.0 - saturate(Roughness));
    float2 RefractedUV = ScreenUV + TangentNormal.xy * DistortionStrength;

    float Level = saturate(Roughness) * 3.0;
    int Level0 = (int)floor(Level);
    int Level1 = min(Level0 + 1, 3);
    float T = frac(Level);

    return lerp(SampleRefractionLevel(Level0, RefractedUV), SampleRefractionLevel(Level1, RefractedUV), T);
}

float4 PSMain(VS_TO_PS In) : SV_Target
{
    GPUMaterialSampledData Material = GPUScene::SampleMaterial(In.MaterialId, In.Uv);

    float3x3 TBN = float3x3(normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal));
    float3 Normal = normalize(mul(Material.Normal, TBN));

    BRDFData BRDF;
    BRDF.N = Normal;
    BRDF.V = -normalize(In.WorldPos - GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz);
    BRDF.Albedo = Material.Albedo;
    BRDF.Roughness = Material.Roughness;
    BRDF.Metallic = Material.Metallic;

    float3 Lighting = EvaluateUnshadowedDirectLighting(BRDF, In.WorldPos);
    Lighting += Material.Emissive;

    if (Material.ShadingMode == MATERIAL_SHADING_REFRACTIVE)
    {
        uint Width;
        uint Height;
        RefractionLevel0.GetDimensions(Width, Height);

        float2 ScreenUV = In.Pos.xy / float2(Width, Height);
        float3 Refracted = SampleRoughRefraction(ScreenUV, Material.Normal, Material.Roughness, Material.Alpha);
        float SurfaceAmount = saturate(Material.Alpha);
        float3 Transmitted = Refracted * lerp(float3(1, 1, 1), Material.Albedo, SurfaceAmount);
        return float4(Transmitted + Lighting * SurfaceAmount, 1.0);
    }

    return float4(Lighting, saturate(Material.Alpha));
}
