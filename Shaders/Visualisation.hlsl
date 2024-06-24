struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

#ifdef VERTEX_SHADER
static const float2 pos[3] =
{
	float2(-1, -1),
	float2(-1, 3),
	float2(3, -1),
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
	Out.Pos = float4(pos[VertexID], 0, 1);
	return Out;
}
#endif

#ifdef PIXEL_SHADER
[[vk::push_constant]]
struct _Params
{
	uint2 ViewportSize;
	uint Flags;
} Params;

#define FLAG_R_ONLY 1
#define FLAG_G_ONLY 2
#define FLAG_NORMAL 4
#define FLAG_DEPTH  8

[[vk::binding(0, 0)]] Texture2D InputTexture;
[[vk::binding(1, 0)]] SamplerState LinearSampler;

float4 Pixel(VS_TO_PS In, float4 Coord : SV_Position) : SV_TARGET
{
	float2 UV = Coord.xy / float2(Params.ViewportSize);

	float4 SourceColour = float4(InputTexture.Sample(LinearSampler, UV).rgb, 1);

	if (Params.Flags == FLAG_R_ONLY)
	{
		return float4(SourceColour.r, SourceColour.r, SourceColour.r, 1);
	}
	else if (Params.Flags == FLAG_G_ONLY)
	{
		return float4(SourceColour.g, SourceColour.g, SourceColour.g, 1);
	}
	else if (Params.Flags == FLAG_NORMAL)
	{
		return float4((SourceColour.rgb + 1) * 0.5, 1);
	}
	else if (Params.Flags == FLAG_DEPTH)
	{
		return float4(SourceColour.rrr, 1);
	}

	return SourceColour;
}
#endif