#pragma pack_matrix(row_major)

#include "GPUScene.hlsli"
#include "Common.hlsli"
#include "SH.hlsli"

[[vk::push_constant]]
struct _Params
{
    uint ObjectId;
} Parameters;

struct VS_TO_PS
{
    float4 Pos              : SV_POSITION;
    float3 Normal           : NORMAL0;
    float2 Uv1              : TEXCOORD0;
    float2 Uv2              : TEXCOORD1;
    float4 ClipspacePos     : POSITION0;
    float4 ClipspacePosPrev : POSITION1;
    float  ViewDepthMotion  : TEXCOORD2;
#if GEOMETRIC_NORMALS
    float3 WorldPosition    : TEXCOORD3;
#endif
    float3 Tangent          : NORMAL1;
    float3 Bitangent        : NORMAL2;
    
    nointerpolation uint MaterialId : COLOR0;
    nointerpolation uint MotionHistoryValid : COLOR1;
    // nointerpolation uint LightmapId : COLOR1;
};

struct PS_Out
{
    float3 Albedo   : SV_Target0;
    float2 Normal   : SV_Target1;
    float2 RM       : SV_Target2;
    float3 Emissive : SV_Target3;
    float4 Velocity : SV_Target4;
    float3 Lightmap : SV_Target5;
#if GEOMETRIC_NORMALS
    float2 GeometricNormal : SV_Target6;
#endif
};

VS_TO_PS VSMain(uint VertexId : SV_VertexID)
{
    VS_TO_PS Out;
    
    GPUSceneMeshCompact Mesh = GPUScene::GPUSceneMeshes[Parameters.ObjectId];

	// index is being read from index buffer by gl_VertexIndex
    GPUScene::Vertex Vertex = GPUScene::FetchVertex(Parameters.ObjectId, VertexId);
    
    float3x3 NormalMatrix = (float3x3) 0;
    NormalMatrix[0] = Mesh.NormalMatrix[0].xyz;
    NormalMatrix[1] = Mesh.NormalMatrix[1].xyz;
    NormalMatrix[2] = Mesh.NormalMatrix[2].xyz;
    
    Out.Normal = normalize(mul(NormalMatrix, Vertex.Normal));
    Out.Tangent = normalize(mul(NormalMatrix, Vertex.TangentAndSign.xyz));
    Out.Bitangent = cross(Out.Normal, Out.Tangent) * Vertex.TangentAndSign.w;

    float4 TransformedPos = mul(Mesh.Transform, float4(Vertex.Position, 1));
    float4 TransformedPosPrev = mul(Mesh.PrevTransform, float4(Vertex.PrevPosition, 1));

    float4 ClipspacePos = mul(GPUScene::GPUSceneScene[0].CameraCur.ViewProjectionMatrix, TransformedPos) * float4(1, -1, 1, 1);
    float4 ClipspacePosPrev = mul(GPUScene::GPUSceneScene[0].CameraPrev.ViewProjectionMatrix, TransformedPosPrev) * float4(1, -1, 1, 1);
    Out.ClipspacePos     = ClipspacePos;
    Out.ClipspacePosPrev = ClipspacePosPrev;
#if GEOMETRIC_NORMALS
    Out.WorldPosition = TransformedPos.xyz;
#endif

    // Compute the difference before interpolation: equal positions produce an
    // exact zero even on thin triangles where raster/interpolated depth differs.
    const float CurrentViewZ  = -mul(GPUScene::GPUSceneScene[0].CameraCur.ViewMatrix, TransformedPos).z;
    const float PreviousViewZ = -mul(GPUScene::GPUSceneScene[0].CameraPrev.ViewMatrix, TransformedPosPrev).z;
    Out.ViewDepthMotion = PreviousViewZ - CurrentViewZ;

    Out.Pos    = ClipspacePos;
    Out.Uv1 = Vertex.UV;
    Out.Uv2 = Vertex.UV2;
    Out.MaterialId = Mesh.MaterialId;
    Out.MotionHistoryValid = Mesh.MotionHistoryValid;
    // Out.LightmapId = Mesh.LightmapId;

    return Out;
}

PS_Out PSMain(VS_TO_PS In)
{
    PS_Out Out;

#if GEOMETRIC_NORMALS
    float3 GeometricNormal = cross(ddy(In.WorldPosition), ddx(In.WorldPosition));
    const float NormalLengthSquared = dot(GeometricNormal, GeometricNormal);
    GeometricNormal = NormalLengthSquared > 1e-20 ? GeometricNormal * rsqrt(NormalLengthSquared) : normalize(In.Normal);

    if (dot(GeometricNormal, GPUScene::GPUSceneScene[0].CameraCur.CameraPosition.xyz - In.WorldPosition) < 0)
    {
        GeometricNormal = -GeometricNormal;
    }

    Out.GeometricNormal = NormalEncode(GeometricNormal);
#endif
    
    float3 LightmapColor = float3(0,0,0);
    // if (In.LightmapId != -1)
    // {
	// 	// TODO: bicubic lightmap sampling
    //     //LightmapColor = textureLod(Textures[InLightmapId], InUV2, 0.0f).rgb;
    // }

#if ALPHA_MASKED
    GPUSceneMaterialCompact SourceMaterial = GPUScene::GPUSceneMaterials[In.MaterialId];
    float4                  AlbedoSample   = GPUScene::SampleTextureWithDefault(SourceMaterial.AlbedoId, In.Uv1, float4(1, 1, 1, 1));
    clip(AlbedoSample.a * SourceMaterial.AlbedoFactor.a - SourceMaterial.AlphaCutoff);
    GPUMaterialSampledData Material = GPUScene::SampleMaterial(SourceMaterial, In.Uv1, AlbedoSample);
#else
    GPUMaterialSampledData Material = GPUScene::SampleMaterial(In.MaterialId, In.Uv1);
#endif

    const bool PreviousValid = In.MotionHistoryValid != 0 && In.ClipspacePosPrev.w > 0;
    float2 Velocity = 0;

    if (PreviousValid)
    {
        float2 NdcCurrent = In.ClipspacePos.xy / In.ClipspacePos.w;
        float2 NdcPrev    = In.ClipspacePosPrev.xy / In.ClipspacePosPrev.w;

		// remove jitter
        NdcCurrent -= GPUScene::GPUSceneScene[0].CameraCur.Jittering  * float2(1, -1);
        NdcPrev    -= GPUScene::GPUSceneScene[0].CameraPrev.Jittering * float2(1, -1);

        Velocity = NdcCurrent - NdcPrev;
    }
    
    // TEST
    LightmapColor = shUnproject(GPUScene::GPUSceneScene[0].SkySHR, GPUScene::GPUSceneScene[0].SkySHG, GPUScene::GPUSceneScene[0].SkySHB, normalize(In.Normal));
    
    float3x3 TBN = float3x3(normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal));


    Out.Albedo = Material.Albedo;
    Out.Normal = NormalEncode(normalize(mul(Material.Normal, TBN)));
    Out.RM = float2(Material.Roughness, Material.Metallic);
    Out.Emissive = Material.Emissive;

    // XY is unjittered NDC motion; Z is previous minus current linear view
    // depth. Invalid previous geometry has a finite FP16 sentinel. W is unused.
    const float DepthMotion = PreviousValid && abs(In.ViewDepthMotion) < MOTION_DEPTH_INVALID ? In.ViewDepthMotion : MOTION_DEPTH_INVALID;
    Out.Velocity = float4(Velocity, DepthMotion, 0);
    Out.Lightmap = LightmapColor;
    
    return Out;
}
