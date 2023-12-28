struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

// TODO: move away from big push constants, use ID + cbuffer
[[vk::push_constant]]
struct _Params {
	float4x4 Model;
	float4x4 VP;
	float4 Colour;
} Params;

static const float3 pos[8] = {
	float3(-1, -1, -1),
	float3(+1, -1, -1),
	float3(+1, -1, +1),
	float3(-1, -1, +1),

	float3(-1, +1, -1),
	float3(+1, +1, -1),
	float3(+1, +1, +1),
	float3(-1, +1, +1),
};

static const int index[36] = {
	0, 1, 2,
	0, 2, 3,

	4, 5, 6,
	4, 6, 7,

	0, 4, 7,
	0, 3, 7,

	1, 5, 6,
	1, 2, 6,

	1, 5, 4,
	1, 0, 4,

	2, 6, 7,
	2, 3, 7,
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
	float4 World = mul(float4(pos[index[VertexID]] * 0.5f, 1), Params.Model);
	Out.Pos = mul(Params.VP, World) * float4(1, -1, 1, 1);
	return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
	return Params.Colour;
}