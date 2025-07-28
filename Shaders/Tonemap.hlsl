struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
	float2 Uv : TEXCOORD;
};

#ifdef VERTEX_SHADER
static const float2 pos[3] = {
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

struct HColourCorrectionSettings
{
	float4 Lift;
	float4 Gamma;
	float4 Gain;
	float4 Offset;
	float Exposure;
	float Contrast;
	float Saturation;
	float HueShift;
	float Temperature;
	float Tint;
	int _pad[10]; // 128
};

[[vk::binding(0, 0)]] RWTexture2D<float4> SceneTexture;
[[vk::binding(1, 0)]] ConstantBuffer<HColourCorrectionSettings> ColourCorrectionSettings;

[[vk::push_constant]]
struct _Params {
	uint FilmCurve;
	uint OutputTransform;
	uint2 Resolution;

	float Vignette;
	float GrainScale;
	float GrainAmount;
	uint  GrainSeed;
} Params;

#define FILMCURVE_ACES 0
#define FILMCURVE_AGX  1
#define FILMCURVE_FLIM 2

#define OUTPUTTRANSFORM_NONE    0
#define OUTPUTTRANSFORM_REC709  1
#define OUTPUTTRANSFORM_REC2020 2

// From BakingLab by MJP
//
// The code in this file was originally written by Stephen Hill (@self_shadow), who deserves all
// credit for coming up with this fit and implementing it. Buy him a beer next time you see him. :)

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
static const float3x3 ACESInputMat =
{
	{0.59719, 0.35458, 0.04823},
	{0.07600, 0.90834, 0.01566},
	{0.02840, 0.13383, 0.83777}
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
static const float3x3 ACESOutputMat =
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
	color = mul(ACESInputMat, color);

	// Apply RRT and ODT
	color = RRTAndODTFit(color);

	color = mul(ACESOutputMat, color);

	// Clamp to [0, 1]
	color = clamp(color, 0, 1);

	// sRGB output
	color = pow(color, float3(0.45, 0.45, 0.45));

	return color;
}

//
//

// AgX
// https://iolite-engine.com/blog_posts/minimal_agx_implementation
//
// 0: Default, 1: Golden, 2: Punchy
#define AGX_LOOK 0

// Mean error^2: 3.6705141e-06
float3 agxDefaultContrastApprox(float3 x)
{
	float3 x2 = x * x;
	float3 x4 = x2 * x2;

	return +15.5 * x4 * x2
		- 40.14 * x4 * x
		+ 31.96 * x4
		- 6.868 * x2 * x
		+ 0.4298 * x2
		+ 0.1191 * x
		- 0.00232;
}

float3 agx(float3 val)
{
	const float3x3 agx_mat = float3x3(
		0.842479062253094, 0.0423282422610123, 0.0423756549057051,
		0.0784335999999992, 0.878468636469772, 0.0784336,
		0.0792237451477643, 0.0791661274605434, 0.879142973793104);

	// TODO: expose black/white points outside

	const float min_ev = -12.47393f; // log2(blackpoint=0.00017578)
	const float max_ev = 4.026069f; // log2(whitepoint=16.2917)

	// Input transform (inset)
	val = mul(agx_mat, val);

	// Log2 space encoding
	val = clamp(log2(val), min_ev, max_ev);
	val = (val - min_ev) / (max_ev - min_ev);

	// Apply sigmoid function approximation
	val = agxDefaultContrastApprox(val);

	return val;
}

float3 agxEotf(float3 val)
{
	const float3x3 agx_mat_inv = float3x3(
		1.19687900512017, -0.0528968517574562, -0.0529716355144438,
		-0.0980208811401368, 1.15190312990417, -0.0980434501171241,
		-0.0990297440797205, -0.0989611768448433, 1.15107367264116);

	// Inverse input transform (outset)
	val = mul(agx_mat_inv, val);

	// sRGB IEC 61966-2-1 2.2 Exponent Reference EOTF Display
	// NOTE: We're linearizing the output here. Comment/adjust when
	// *not* using a sRGB render target
	val = pow(val, float3(2.2, 2.2, 2.2));

	return val;
}

float3 agxLook(float3 val)
{
	const float3 lw = float3(0.2126, 0.7152, 0.0722);
	float luma = dot(val, lw);

	// TODO: how about exposing those values outside?

	// Default
	float3 offset = float3(0, 0, 0);
	float3 slope = float3(1, 1, 1);
	float3 power = float3(1, 1, 1);
	float sat = 1.0;

#if AGX_LOOK == 1
	// Golden
	slope = float3(1.0, 0.9, 0.5);
	power = float3(0.8, 0.8, 0.8);
	sat = 0.8;
#elif AGX_LOOK == 2
	// Punchy
	slope = float3(1.0, 1.0, 1.0);
	power = float3(1.35, 1.35, 1.35);
	sat = 1.4;
#endif

	// ASC CDL
	val = pow(val * slope + offset, power);
	return luma + sat * (val - luma);
}

float3 Tonemap(float3 Linear)
{
	switch (Params.FilmCurve)
	{
	case FILMCURVE_ACES:
	{	
		// TODO: proper ACES
		return ACESFitted(Linear); // ACES RRT + sRGB ODT
	}
	case FILMCURVE_AGX:
	{
		float3 a = agx(Linear);
		a = agxLook(a);
		a = agxEotf(a);
		return a;
	}

	case FILMCURVE_FLIM: // TODO:
	default:
		return Linear;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 
// Got from AMD FidelityFX Lens
//
// Noise function used as basis for film grain effect
uint3 pcg3d16(uint3 v)
{
	v = v * 12829u + 47989u;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.x += v.y * v.z;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v >>= 16u;
	return v;
}

// Simplex noise, transforms given position onto triangle grid
// This logic should be kept at 32-bit floating point precision. 16 bits causes artifacting.
float2 simplex(const in float2 P)
{
	// Skew and unskew factors are a bit hairy for 2D, so define them as constants
	//const float F2 = (sqrt(3.0) - 1.0) / 2.0;  // 0.36602540378
	//const float G2 = (3.0 - sqrt(3.0)) / 6.0;  // 0.2113248654
	const float F2 = 0.36602540378;
	const float G2 = 0.2113248654;

	// Skew the (x,y) space to determine which cell of 2 simplices we're in
	float   u = (P.x + P.y) * F2;
	float2 Pi = round(P + u);
	float   v = (Pi.x + Pi.y) * G2;
	float2 P0 = Pi - v;  // Unskew the cell origin back to (x,y) space
	float2 Pf0 = P - P0;  // The x,y distances from the cell origin

	return float2(Pf0);
}

float2 toFloat16(float2 inputVal)
{
	return float2(inputVal * (1.0 / 65536.0) - 0.5);
}

float3 toFloat16(float3 inputVal)
{
	return float3(inputVal * (1.0 / 65536.0) - 0.5);
}

void FfxLensApplyFilmGrain(int2 coord, inout float3 color, float grainScaleVal, float grainAmountVal, uint grainSeedVal)
{
	float2     randomNumberFine = toFloat16(pcg3d16(float3(coord / (grainScaleVal / 8), grainSeedVal)).xy).xy;
	float2     simplexP = simplex(coord / grainScaleVal + randomNumberFine);
	const float grainShape = 3;

	float grain = 1 - 2 * exp2(-length(simplexP) * grainShape);

	color += grain * min(color, 1 - color) * grainAmountVal;
}

void FfxLensApplyVignette(int2 coord, int2 centerCoord, inout float3 color, float vignetteAmount)
{
	float2 vignetteMask = float2(0.0, 0.0);
	float2 coordFromCenter = abs(coord - centerCoord) / float2(centerCoord);

	const float piOver4 = 3.141592659 * 0.25;
	vignetteMask = cos(coordFromCenter * vignetteAmount * piOver4);
	vignetteMask = vignetteMask * vignetteMask;
	vignetteMask = vignetteMask * vignetteMask;

	color *= clamp(vignetteMask.x * vignetteMask.y, 0, 1);
}

///////////////////////////////////////////////////////////////////////////////
// 
// Colour correction

float3 ApplyLiftGammaGain(float3 color)
{
	color = ColourCorrectionSettings.Lift.rgb + color * ColourCorrectionSettings.Gain.rgb;
	color = pow(abs(color), 1.0f / max(ColourCorrectionSettings.Gamma.rgb, 0.001f));
	color += ColourCorrectionSettings.Offset.rgb;
	return color;
}

float3 ApplySaturation(float3 color)
{
	float luminance = dot(color, float3(0.299, 0.587, 0.114));
	return lerp(float3(luminance, luminance, luminance), color, ColourCorrectionSettings.Saturation);
}

// Approximate hue shift with rotation around grayscale axis
float3 ApplyHueShift(float3 color)
{
	float angle = radians(ColourCorrectionSettings.HueShift);
	float s = sin(angle);
	float c = cos(angle);
	const float3x3 hueRot =
	{
		{0.213f + c * 0.787f - s * 0.213f, 0.715f - c * 0.715f - s * 0.715f, 0.072f - c * 0.072f + s * 0.928f},
		{0.213f - c * 0.213f + s * 0.143f, 0.715f + c * 0.285f + s * 0.140f, 0.072f - c * 0.072f - s * 0.283f},
		{0.213f - c * 0.213f - s * 0.787f, 0.715f - c * 0.715f + s * 0.715f, 0.072f + c * 0.928f + s * 0.072f}
	};
	return mul(color, hueRot);
}

float3 ApplyExposure(float3 color)
{
	return color * exp2(ColourCorrectionSettings.Exposure);
}

float3 ApplyContrast(float3 color)
{
	return 0.5 + (color - 0.5) * ColourCorrectionSettings.Contrast;
}

float3 KelvinToRGB(float kelvin)
{
	kelvin = kelvin / 100.0;

	float r = kelvin <= 66 ? 1.0 : saturate(1.292936186f * pow(kelvin - 60.0, -0.1332047592f));
	float g = kelvin <= 66 ? saturate(0.3900815788f * log(kelvin) - 0.6318414438f)
						   : saturate(1.1298908609f * pow(kelvin - 60.0, -0.0755148492f));
	float b = kelvin >= 66 ? 1.0 : kelvin <= 19 ? 0.0
						   : saturate(0.5432067891f * log(kelvin - 10.0) - 1.1962540891f);

	return float3(r, g, b);
}

float3 ApplyWhiteBalance(float3 color)
{
	// Normalize Kelvin to D65 white point (6500K)
	float3 whiteBalance = KelvinToRGB(ColourCorrectionSettings.Temperature) / KelvinToRGB(6500.0);
	color *= whiteBalance;

	// Tint adjustment: green-magenta axis
	float tint = ColourCorrectionSettings.Tint * 0.01f; // [-1..1] scale (or adjust based on UI range)
	float3 tintBalance = float3(1.0, 1.0 - tint, 1.0 + tint);
	color *= tintBalance;

	return color;
}

float4 main(VS_TO_PS Input) : SV_TARGET
{
	// ACES image formation recap:
	// Apply Input Transform to convert footage from internal format (Linear sRGB D65) to ACES AP1 (D60)
	// Modify colours
	// Apply RRT
	// Apply Output Transform (Rec709 or Rec2020-PQ)

	float3 Linear = float3(Input.Uv*5, 0);
	int2 Coord = Input.Uv * Params.Resolution;

	Linear = SceneTexture[Coord].rgb;

	// do colour correction
	Linear = ApplyExposure(Linear);
	Linear = ApplyWhiteBalance(Linear);
	Linear = ApplyLiftGammaGain(Linear);
	Linear = ApplyHueShift(Linear);
	Linear = ApplySaturation(Linear);
	Linear = ApplyContrast(Linear);

	float3 Tonemapped =  Tonemap(Linear);

	FfxLensApplyVignette(Coord, Params.Resolution/2, Tonemapped, Params.Vignette);
	FfxLensApplyFilmGrain(Coord, Tonemapped, Params.GrainScale, Params.GrainAmount, Params.GrainSeed);

	return float4(Tonemapped, 1);
}
#endif
