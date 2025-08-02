[[vk::push_constant]]
struct _Params
{
	float4x4 ViewProjection;
	float4x4 BillboardMatrix;
    int2 Frame;
} Params;

struct GPUParticleCompact
{
	float4 Position_Rotation;
	float4 Size_Frame;
	float4 Colour;
};

StructuredBuffer<GPUParticleCompact> Particles : register(t0);
Texture2D<float4> Texture : register(t1);
SamplerState Sampler : register(s2);

struct VS_TO_PS
{
	float4 Pos    : SV_POSITION;
	float4 Colour : COLOR;
	float2 UV     : TEXCOORD0;
};

static const float2 Coord[6] =
{
	float2(-1, +1),
	float2(+1, +1),
	float2(+1, -1),
	float2(-1, -1),
	float2(-1, +1),
	float2(+1, -1)
};

static const float2 UV[6] =
{
	float2(0, 1),
	float2(1, 1),
	float2(1, 0),
	float2(0, 0),
	float2(0, 1),
	float2(1, 0)
};

float2 ExtractSubUV(int FrameNumber, float2 Texcoords, int2 Frame)
{
    int FrameHorizontal = FrameNumber % int(Frame.x);
    int FrameVertical = FrameNumber / int(Frame.x) - 1;
    float2 Begin = 1.0 / float2(Frame);

    return float2(Texcoords.x * Begin.x + Begin.x * FrameHorizontal,
	              Texcoords.y * Begin.y + Begin.y * FrameVertical);
}

float4x4 RotationMatrix(float3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;

	return  float4x4(oc * axis.x * axis.x + c,       oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
				 oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
				 oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
				 0.0,                                0.0,                                0.0,                                1.0);
}

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
	VS_TO_PS Out;
	
	int VertexIndex = VertexID % 6;
	int ParticleID  = VertexID / 6;
	GPUParticleCompact Particle = Particles[ParticleID];
	
	float4x4 Rotation = RotationMatrix(float3(0, 0, 1), 0.011111 * Particle.Position_Rotation.w);
	float4 VertexPosition = float4(float3(Coord[VertexIndex], 0) * Particle.Size_Frame.xyz, 1);
    float4 WorldPosition = float4(Particle.Position_Rotation.xyz, 0);
	
    Out.Pos = mul(mul(Params.BillboardMatrix, mul(VertexPosition, Rotation)) + WorldPosition, Params.ViewProjection) * float4(1, -1, 1, 1);
	//Out.UV = UV[VertexIndex];
    Out.UV = ExtractSubUV(int(Particle.Size_Frame.w), UV[VertexIndex], Params.Frame);
	Out.Colour = Particle.Colour;

	return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{	
    return In.Colour * Texture.Sample(Sampler, In.UV);
}