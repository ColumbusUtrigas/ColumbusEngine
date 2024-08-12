// TODO: unify computations with path tracing (medium, requires full HLSL for PT)

#include "SkyCommon.hlsli"

[[vk::push_constant]]
struct _Params
{
    float4x4 InverseViewProjection; // camera-space to world-space transform
    float4   CameraPosition;
    float4   SunDirection;
} Params;

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
    Out.Pos = float4(pos[VertexID], 0.9999, 1);
    Out.Pos2d = Out.Pos.xy;
    return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{   
    // z=-1 for "forward" direction, -1-1 NDC xy position on a far plane
    float4 ViewDirectionCameraSpace = float4(In.Pos2d.xy * float2(1, -1), -1, 1);
    float4 ViewDirectionWorldSpace = mul(ViewDirectionCameraSpace, Params.InverseViewProjection);
    ViewDirectionWorldSpace /= ViewDirectionWorldSpace.w; // perspective divide

    float3 ViewDirection = normalize(ViewDirectionWorldSpace.xyz - Params.CameraPosition.xyz);
    float3 CameraPosition = Params.CameraPosition.xyz;
    float3 SunDirection = normalize(Params.SunDirection.xyz);

    return float4(Sky::Atmosphere(CameraPosition, ViewDirection, SunDirection), 1);

    //return float4(0.412, 0.796, 1.0, 1);
}