struct VS_TO_PS
{
    float4 Pos : SV_POSITION;
    float2 Uv : TEXCOORD0;
};

[[vk::binding(0, 0)]] Texture2D<float4> Texture;
[[vk::binding(1, 0)]] SamplerState Sampler;

[[vk::push_constant]]
struct _Params
{
    float4 Colour;
    float2 Position;
    float2 Size;
} Params;

static const float2 pos[6] =
{
    float2(-1, -1),
	float2(-1, +1),
	float2(+1, -1),

    float2(+1, -1),
    float2(-1, +1),
	float2(+1, +1),
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
    float2 vertex = pos[VertexID];

    VS_TO_PS Out;
    Out.Pos = float4(vertex * Params.Size + Params.Position, 0, 1) * float4(1, -1, 1, 1);
    Out.Uv = vertex * 0.5f + 0.5f;
    return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
    In.Uv.y = 1 - In.Uv.y;
    return Texture.Sample(Sampler, In.Uv) * Params.Colour;
}