#include "Common.hlsli"
#include "SkyCommon.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"

[[vk::binding(0, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

struct VS_TO_PS
{
	float4 Pos   : SV_POSITION;
    float2 Pos2d : TEXCOORD0;
};

static const float2 pos[3] = {
	float2(-1, -1),
	float2(-1, +3),
	float2(+3, -1)
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
    VS_TO_PS Out;
    Out.Pos = float4(pos[VertexID], DEVICE_DEPTH_FAR, 1);
    Out.Pos2d = Out.Pos.xy;
    return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{   
    float4 ViewDirectionCameraSpace = float4(ScreenUVToNDC(In.Pos2d.xy * 0.5 + 0.5), DEVICE_DEPTH_FAR, 1);
    float4 ViewDirectionWorldSpace = mul(GPUSceneScene[0].CameraCur.InverseViewProjectionMatrix, ViewDirectionCameraSpace);
    ViewDirectionWorldSpace /= ViewDirectionWorldSpace.w; // perspective divide

    float3 CameraPosition = GPUSceneScene[0].CameraCur.CameraPosition;
    float3 ViewDirection = normalize(ViewDirectionWorldSpace.xyz - CameraPosition);
    float3 SunDirection = normalize(GPUSceneScene[0].SunDirection.xyz);

    return float4(Sky::Atmosphere(CameraPosition, ViewDirection, SunDirection, GPUSceneScene[0].Sky), 1);

    //return float4(0.412, 0.796, 1.0, 1);
}
