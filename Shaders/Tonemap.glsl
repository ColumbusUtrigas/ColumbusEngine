#version 460 core

#ifdef VERTEX_SHADER
const vec2 pos[3] = {
	vec2(-1, -1),
	vec2(-1, 3),
	vec2(3, -1),
};

layout(location=0) out vec2 UV;

void main()
{
	UV = (pos[gl_VertexIndex] + 1) / 2;
	gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
}
#endif

#ifdef PIXEL_SHADER
layout(location=0) out vec4 RT0;

layout(location=0) in vec2 UV;


// From BakingLab by MJP
//
// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat =
{
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat =
{
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    // Clamp to [0, 1]
    color = clamp(color, 0, 1);

	// sRGB output
	color = pow(color, vec3(0.45));

    return color;
}

vec3 Tonemap(vec3 Linear)
{
	return ACESFitted(Linear); // ACES RRT + sRGB ODT
}

void main()
{
	// ACES image formation recap:
	// Apply Input Transform to convert footage from internal format (Linear sRGB D65) to ACES AP1 (D60)
	// Modify colours
	// Apply RRT
	// Apply Output Transform (Rec709 or Rec2020-PQ)

	vec3 Linear = vec3(UV*5, 0);

	RT0 = vec4(Tonemap(Linear),1);
}
#endif
