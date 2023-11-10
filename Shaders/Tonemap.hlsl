struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
	float2 Uv  : TEXCOORD;
};

#ifdef VERTEX_SHADER
const float2 pos[3] = {
	float2(-1, -1),
	float2(-1, 3),
	float2(3, -1),
};

VS_TO_PS main(uint VertexID : SV_VertexID)
{
	VS_TO_PS Output;
	Output.Pos = float4(pos[VertexID], 0, 1);
	Output.Uv = (pos[VertexID] + 1) / 2;
	return Output;
}
#endif

#ifdef PIXEL_SHADER
RWTexture2D<float4> SceneTexture;

[[vk::push_constant]]
cbuffer Params {
	uint FilmCurve;
	uint OutputTransform;
	uint2 Resolution;
};

// From BakingLab by MJP
//
// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const float3x3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const float3x3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color)
{
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    // Clamp to [0, 1]
    color = clamp(color, 0, 1);

	// sRGB output
	color = pow(color, float3(0.45));

    return color;
}

float3 Tonemap(float3 Linear)
{
	return ACESFitted(Linear); // ACES RRT + sRGB ODT
}

float4 main(VS_TO_PS Input) : SV_TARGET
{

	// ACES image formation recap:
	// Apply Input Transform to convert footage from internal format (Linear sRGB D65) to ACES AP1 (D60)
	// Modify colours
	// Apply RRT
	// Apply Output Transform (Rec709 or Rec2020-PQ)

	float3 Linear = float3(Input.Uv*5, 0);
	Linear = SceneTexture[uint2(Input.Uv * Resolution)].rgb;

	return float4(Tonemap(Linear),1);
}
#endif
