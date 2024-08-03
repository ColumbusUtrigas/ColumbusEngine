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

	float4 Vertices[3];
	uint Type;
} Params;

#define TYPE_BOX 0
#define TYPE_TRI 1

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
	float4 vertex = float4(pos[index[VertexID]] * 0.5f, 1);

	if (Params.Type == TYPE_TRI)
	{
		vertex = Params.Vertices[VertexID];
	}

	VS_TO_PS Out;
	float4 World = mul(vertex, Params.Model);
	Out.Pos = mul(World, Params.VP) * float4(1, -1, 1, 1);
	return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
	return Params.Colour;
}