#pragma pack_matrix(row_major)

#define IRRADIANCE_PROBES_SET 0
#include "IrradianceVolumeCommon.hlsli"

struct VS_TO_PS
{
    float4 Pos : SV_POSITION;
    float2 Uv : TEXCOORD0;
    nointerpolation uint ProbeId : TEXCOORD1;
    nointerpolation float3 Right : TEXCOORD2;
    nointerpolation float3 Up : TEXCOORD3;
    nointerpolation float3 Forward : TEXCOORD4;
};

[[vk::push_constant]]
struct _Params
{
    float4x4 View;
    float4x4 Projection;
    float4 Position;
    float4 Extent;
    int4 ProbesCount;
    int4 ProbeIndex;
    float4 TestPoint;
} Params;

static const float2 Quad[6] =
{
    float2(-1.0, -1.0),
    float2( 1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0, -1.0),
    float2( 1.0,  1.0),
    float2(-1.0,  1.0)
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
    uint ProbeId = VertexID / 6;
    uint CornerId = VertexID % 6;

    IrradianceProbe Probe = IrradianceProbes[ProbeId];

    float3 Right = normalize(Params.Position.xyz);
    float3 Forward = normalize(Params.TestPoint.xyz);
    float3 Up = normalize(cross(Forward, Right));
    float CellMin = min(Params.Extent.x / max(Params.ProbesCount.x, 1), min(Params.Extent.y / max(Params.ProbesCount.y, 1), Params.Extent.z / max(Params.ProbesCount.z, 1)));
    float Radius = max(CellMin * 0.08, 0.03) * max(Params.TestPoint.w, 0.01);
    float2 Offset = Quad[CornerId] * Radius;
    float3 WorldPosition = Probe.PositionState.xyz + Right * Offset.x + Up * Offset.y;

    VS_TO_PS Out;
    float4 ViewPosition = mul(Params.View, float4(WorldPosition, 1.0));
    Out.Pos = mul(Params.Projection, ViewPosition) * float4(1, -1, 1, 1);
    Out.Uv = Quad[CornerId];
    Out.ProbeId = ProbeId;
    Out.Right = Right;
    Out.Up = Up;
    Out.Forward = Forward;
    return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
    float RadiusSq = dot(In.Uv, In.Uv);
    clip(1.0 - RadiusSq);

    float Z = sqrt(saturate(1.0 - RadiusSq));
    float3 Normal = normalize(In.Right * In.Uv.x + In.Up * In.Uv.y + In.Forward * Z);
    IrradianceProbe Probe = IrradianceProbes[In.ProbeId];
    float3 Colour = Probe.PositionState.w > 0.5 ? SampleIrradianceProbe(Probe, Normal) : float3(1.0, 0.05, 0.0);
    return float4(Colour, 1.0);
}
