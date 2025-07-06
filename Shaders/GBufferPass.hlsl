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
    float3x3 TBN            : POSITION3;
    
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

	// TODO: precompute and store this matrix in the buffer
    //float3x3 NormalMatrix = transpose(inverse(transpose(float3x3(Mesh.Transform))));
    float3x3 NormalMatrix = float3x3(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1));
    NormalMatrix = transpose((float3x3) Mesh.Transform);
    float3 Normal    = normalize(mul(NormalMatrix, Vertex.Normal));
    float3 Tangent   = normalize(mul(NormalMatrix, Vertex.TangentAndSign.xyz));
    float3 Bitangent = cross(Normal, Tangent) * Vertex.TangentAndSign.w;

    Out.TBN = float3x3(Tangent, Bitangent, Normal);

	// TODO: fucked up row/column major?
    float4 TransformedPos = mul(float4(Vertex.Position, 1), Mesh.Transform);
    Out.WorldPos = TransformedPos.xyz;
		
    float4 ClipspacePos     = mul(TransformedPos, GPUScene::GPUSceneScene[0].CameraCur.ViewProjectionMatrix)  * float4(1, -1, 1, 1);
    float4 ClipspacePosPrev = mul(TransformedPos, GPUScene::GPUSceneScene[0].CameraPrev.ViewProjectionMatrix) * float4(1, -1, 1, 1);
    Out.ClipspacePos     = ClipspacePos;
    Out.ClipspacePosPrev = ClipspacePosPrev;

    Out.Pos    = ClipspacePos;
    Out.Normal = Normal;
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

    Out.Albedo = Material.Albedo;
	// RT1 = normalize(TBN * Material.Normal);
    Out.Normal = normalize(In.Normal);
    Out.WP = In.WorldPos; // TODO: remove? reconstruct from screenpos and linear depth
    Out.RM = float2(Material.Roughness, Material.Metallic);
    Out.Velocity = Velocity;
    Out.Lightmap = LightmapColor;
    
    return Out;
}