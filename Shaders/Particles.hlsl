[[vk::push_constant]]
struct _Params
{
	float4x4 ViewProjection;
	float4x4 BillboardMatrix;
    int2 SubImages;
    float SubImagesSpeed;
    float MotionVectorStrength; // < 0 if no motion vectors
} Params;

struct GPUParticleCompact
{
	float4 Position_Rotation;
	float4 Size_Age; // xyz - size, w - normalised age
	float4 Colour;
};

[[vk::binding(0, 0)]] StructuredBuffer<GPUParticleCompact> Particles : register(t0);
[[vk::binding(1, 0)]] Texture2D<float4> Texture                      : register(t1);
[[vk::binding(2, 0)]] Texture2D<float4> MotionVectors                : register(t2);
[[vk::binding(3, 0)]] SamplerState Sampler                           : register(s3);

struct VS_TO_PS
{
	float4 Pos    : SV_POSITION;
	float4 Colour : COLOR;
	float2 UV     : TEXCOORD0;
    float  Age    : TEXCOORD1;
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

// Convert (uv in [0,1], int index) to atlas UV
float2 SubUV(float2 uv, uint frame, float2 subImages)
{
    float2 size = 1.0 / subImages;
    uint cols = (uint) subImages.x;
    uint idx = frame % (uint) (subImages.x * subImages.y);
    uint cellY = idx / cols;
    uint cellX = idx - cellY * cols;
    return uv * size + float2(cellX, cellY) * size;
}

// Sample RG flow from a given frame and convert to a signed offset
float2 SampleFlow(float2 uv, uint frame)
{
    float2 atlasUV = SubUV(uv, frame, Params.SubImages);
    float2 rg = MotionVectors.Sample(Sampler, atlasUV).rg;
    float2 flow = rg * 2.0 - 1.0;
    return flow * Params.MotionVectorStrength;
}

float4 SampleSprite(float2 uv, uint frame, float2 flowOffset)
{
    float2 atlasUV = SubUV(uv + flowOffset, frame, Params.SubImages);
    return Texture.Sample(Sampler, atlasUV);
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
	float4 VertexPosition = float4(float3(Coord[VertexIndex], 0) * Particle.Size_Age.xyz, 1);
    float4 WorldPosition = float4(Particle.Position_Rotation.xyz, 0);
	
    Out.Pos = mul(mul(Params.BillboardMatrix, mul(VertexPosition, Rotation)) + WorldPosition, Params.ViewProjection) * float4(1, -1, 1, 1);
	Out.UV = UV[VertexIndex];
	Out.Colour = Particle.Colour;
    Out.Age = Particle.Size_Age.w;

	return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{	
    float fFrame = frac(In.Age * Params.SubImagesSpeed) * Params.SubImages.x * Params.SubImages.y;
    uint frameA = (uint) floor(fFrame);
	
    if (Params.MotionVectorStrength > 0.0f)
    {
        uint frameB = frameA + 1u;
        float alpha = frac(fFrame);
		
		// flow from current and next frames
        float2 flowA = SampleFlow(In.UV, frameA);
        float2 flowB = SampleFlow(In.UV, frameB);
        float2 flow = lerp(flowA, flowB, alpha);
		
		// distorted sprite samples at current and next frames, then frame blend
        float4 spriteA = SampleSprite(In.UV, frameA, flow);
        float4 spriteB = SampleSprite(In.UV, frameB, flow);
        float4 sprite = lerp(spriteA, spriteB, alpha);
		
        return In.Colour * sprite;
    }
	else
    {
		// simpler path without motion vectors
        return In.Colour * Texture.Sample(Sampler, SubUV(In.UV, frameA, Params.SubImages));
    }
}