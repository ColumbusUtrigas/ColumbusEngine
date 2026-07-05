#pragma pack_matrix(row_major)

#include "../Common.hlsli"

static const uint SPHERE_SLICES = 32;
static const uint SPHERE_STACKS = 16;

struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
	float3 WorldPosition : TEXCOORD0;
	float3 WorldNormal : TEXCOORD1;
};

[[vk::push_constant]]
struct _Params
{
	float4x4 ViewProjection;
	float4 CapturePositionAndRadius;
	float4 CameraPositionAndRoughness;
	float4 CaptureIntensity;
	int4 CaptureIndexAndCount;
} Params;

float3 SpherePosition(uint VertexID)
{
	uint CellIndex = VertexID / 6;
	uint VertexInCell = VertexID % 6;

	uint Slice = CellIndex % SPHERE_SLICES;
	uint Stack = CellIndex / SPHERE_SLICES;

	float U0 = (float)Slice / SPHERE_SLICES;
	float U1 = (float)(Slice + 1) / SPHERE_SLICES;
	float V0 = (float)Stack / SPHERE_STACKS;
	float V1 = (float)(Stack + 1) / SPHERE_STACKS;

	float2 UV;
	if (VertexInCell == 0) UV = float2(U0, V0);
	else if (VertexInCell == 1) UV = float2(U1, V0);
	else if (VertexInCell == 2) UV = float2(U1, V1);
	else if (VertexInCell == 3) UV = float2(U0, V0);
	else if (VertexInCell == 4) UV = float2(U1, V1);
	else UV = float2(U0, V1);

	float Lon = UV.x * 2.0 * PI;
	float Lat = UV.y * PI;

	return float3(sin(Lat) * cos(Lon), cos(Lat), sin(Lat) * sin(Lon));
}

#ifdef VERTEX_SHADER
VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	float3 LocalPosition = SpherePosition(VertexID);
	float Radius = max(Params.CapturePositionAndRadius.w, 0.001);
	float3 WorldPosition = Params.CapturePositionAndRadius.xyz + LocalPosition * Radius;

	VS_TO_PS Out;
	Out.WorldPosition = WorldPosition;
	Out.WorldNormal = LocalPosition;
	Out.Pos = mul(Params.ViewProjection, float4(WorldPosition, 1.0)) * float4(1, -1, 1, 1);
	return Out;
}
#endif

#ifdef PIXEL_SHADER
[[vk::binding(0, 0)]] SamplerState LinearSampler;
[[vk::binding(1, 0)]] Texture2D<float2> EnvBRDFLUT;
[[vk::binding(2, 0)]] TextureCube<float4> ReflectionCaptureRadiance[32];

#include "EnvBRDF.hlsli"

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
	int CaptureIndex = clamp(Params.CaptureIndexAndCount.x, 0, 31);
	float3 Normal = normalize(In.WorldNormal);
	float3 View = normalize(Params.CameraPositionAndRoughness.xyz - In.WorldPosition);
	float3 Reflection = reflect(-View, Normal);
	float Roughness = saturate(Params.CameraPositionAndRoughness.w);
	float Mip = EnvPrefilteredMip(Roughness, (float)Params.CaptureIndexAndCount.z);
	float NoV = saturate(dot(Normal, View));

	float3 Colour = ReflectionCaptureRadiance[NonUniformResourceIndex(CaptureIndex)].SampleLevel(LinearSampler, Reflection, Mip).rgb;
	Colour = ApplyEnvBRDF(EnvBRDFLUT, LinearSampler, Colour * Params.CaptureIntensity.x, float3(1.0, 1.0, 1.0), Roughness, NoV);
	Colour = Colour / (1.0 + Colour);
	return float4(Colour, 1.0);
}
#endif
