#include "GPUScene.hlsli"
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
    float3 WorldPos         : POSITION0;
    float4 ClipspacePos     : POSITION1;
    float4 ClipspacePosPrev : POSITION2;
    float3 Tangent          : NORMAL1;
    float3 Bitangent        : NORMAL2;
    
    nointerpolation uint MaterialId : COLOR0;
    nointerpolation uint LightmapId : COLOR1;
};

struct PS_Out
{
    float3 Albedo   : SV_Target0;
    float3 Normal   : SV_Target1;
    float3 WP       : SV_Target2;
    float2 RM       : SV_Target3;
    float2 Velocity : SV_Target4;
    float3 Lightmap : SV_Target5;
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
    
    Out.Normal = normalize(mul(Vertex.Normal, NormalMatrix));
    Out.Tangent = normalize(mul(Vertex.TangentAndSign.xyz, NormalMatrix));
    Out.Bitangent = cross(Out.Normal, Out.Tangent) * Vertex.TangentAndSign.w;

	// TODO: fucked up row/column major?
    float4 TransformedPos = mul(float4(Vertex.Position, 1), Mesh.Transform);
    Out.WorldPos = TransformedPos.xyz;
		
    float4 ClipspacePos     = mul(TransformedPos, GPUScene::GPUSceneScene[0].CameraCur.ViewProjectionMatrix)  * float4(1, -1, 1, 1);
    float4 ClipspacePosPrev = mul(TransformedPos, GPUScene::GPUSceneScene[0].CameraPrev.ViewProjectionMatrix) * float4(1, -1, 1, 1);
    Out.ClipspacePos     = ClipspacePos;
    Out.ClipspacePosPrev = ClipspacePosPrev;

    Out.Pos    = ClipspacePos;
    Out.Uv1 = Vertex.UV;
    Out.Uv2 = Vertex.UV2;
    Out.MaterialId = Mesh.MaterialId;
    Out.LightmapId = Mesh.LightmapId;

    return Out;
}

PS_Out PSMain(VS_TO_PS In)
{
    PS_Out Out;
    
    float3 LightmapColor = float3(0,0,0);
    if (In.LightmapId != -1)
    {
		// TODO: bicubic lightmap sampling
        //LightmapColor = textureLod(Textures[InLightmapId], InUV2, 0.0f).rgb;
    }

    GPUMaterialSampledData Material = GPUScene::SampleMaterial(In.MaterialId, In.Uv1);

    float2 Velocity;
	{
        float2 NdcCurrent = In.ClipspacePos.xy / In.ClipspacePos.w;
        float2 NdcPrev    = In.ClipspacePosPrev.xy / In.ClipspacePosPrev.w;

		// remove jitter
        NdcCurrent -= GPUScene::GPUSceneScene[0].CameraCur.Jittering  * float2(1, -1);
        NdcPrev    -= GPUScene::GPUSceneScene[0].CameraPrev.Jittering * float2(1, -1);

        Velocity = NdcCurrent - NdcPrev;
		//Velocity = NdcToUv(NdcCurrent) - NdcToUv(NdcPrev);
    }
    
    // TEST
    LightmapColor = shUnproject(GPUScene::GPUSceneScene[0].SkySHR, GPUScene::GPUSceneScene[0].SkySHG, GPUScene::GPUSceneScene[0].SkySHB, normalize(In.Normal));
    
    float3x3 TBN = float3x3(normalize(In.Tangent), normalize(In.Bitangent), normalize(In.Normal));

    Out.Albedo = Material.Albedo;
    Out.Normal = normalize(mul(Material.Normal, TBN));
    Out.WP = In.WorldPos; // TODO: remove? reconstruct from screenpos and linear depth
    Out.RM = float2(Material.Roughness, Material.Metallic);
    Out.Velocity = Velocity;
    Out.Lightmap = LightmapColor;
    
    return Out;
}