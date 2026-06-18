#pragma once
#pragma pack_matrix(row_major)

#define GOLDEN_RATIO 1.618033988749894
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI 0.318310
#define EPSILON 0.0001
#define DEVICE_DEPTH_FAR 0.0
#define DEVICE_DEPTH_NEAR 1.0
#define DEVICE_DEPTH_SKY_EPSILON 0.0000001

bool IsSkyDepth(float depth)
{
	return depth <= DEVICE_DEPTH_SKY_EPSILON;
}

bool IsSceneDepth(float depth)
{
	return !IsSkyDepth(depth);
}

///////////////////////////////////////////////////////////////////
// Math
// Orthonormal basis from single direction
// https://backend.orbit.dtu.dk/ws/portalfiles/portal/126824972/onb_frisvad_jgt2012_v2.pdf
// https://graphics.pixar.com/library/OrthonormalB/paper.pdf
float3x3 ComputeTangentsFromVector(float3 normal)
{
	// Using right-hand coord
	const float3 up = abs(normal.y) < 0.999 ? float3(0.0, 1.0, 0.0) : float3(1.0, 0.0, 0.0);
	const float3 xAxis = normalize(cross(up, normal));
	const float3 yAxis = cross(normal, xAxis);
	const float3 zAxis = normal;

	return float3x3(xAxis, yAxis, zAxis);
}

float2 ScreenUVToNDC(float2 uv)
{
	return (uv * 2.0 - 1.0) * float2(1.0, -1.0);
}

float2 NDCToScreenUV(float2 ndc)
{
	return ndc * float2(0.5, -0.5) + 0.5;
}

float2 ClipToScreenUV(float4 clip)
{
	return NDCToScreenUV(clip.xy / clip.w);
}

float2 PixelToScreenUV(float2 pixel, float2 size)
{
	return (pixel + 0.5) / size;
}

float2 PixelToNDC(float2 pixel, float2 size)
{
	return ScreenUVToNDC(PixelToScreenUV(pixel, size));
}

float3 ReconstructViewPositionFromDepthNDC(float2 ndc, float depth, float4x4 inverseProjection)
{
    float4 position = mul(inverseProjection, float4(ndc, depth, 1.0));
	return position.xyz / position.w;
}

float3 ReconstructViewPositionFromDepth(float2 uv, float depth, float4x4 inverseProjection)
{
	return ReconstructViewPositionFromDepthNDC(ScreenUVToNDC(uv), depth, inverseProjection);
}

float3 ReconstructViewPositionFromDepth(uint2 pixel, float depth, uint2 size, float4x4 inverseProjection)
{
	return ReconstructViewPositionFromDepthNDC(PixelToNDC((float2)pixel, (float2)size), depth, inverseProjection);
}

float3 ReconstructWorldPositionFromDepthNDC(float2 ndc, float depth, float4x4 inverseViewProjection)
{
    float4 position = mul(inverseViewProjection, float4(ndc, depth, 1.0));
	return position.xyz / position.w;
}

float3 ReconstructWorldPositionFromDepth(float2 uv, float depth, float4x4 inverseViewProjection)
{
	return ReconstructWorldPositionFromDepthNDC(ScreenUVToNDC(uv), depth, inverseViewProjection);
}

float3 ReconstructWorldPositionFromDepth(uint2 pixel, float depth, uint2 size, float4x4 inverseViewProjection)
{
	return ReconstructWorldPositionFromDepthNDC(PixelToNDC((float2)pixel, (float2)size), depth, inverseViewProjection);
}

// Math
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Normals

// Octahedron normals encoding
// https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/

float2 NormalOctWrap(float2 v)
{
	float2 wrappedSign = float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0);
	return (1.0 - abs(v.yx)) * wrappedSign;
}

float2 NormalEncode(float3 n)
{
	n /= (abs(n.x) + abs(n.y) + abs(n.z));
	n.xy = n.z >= 0.0 ? n.xy : NormalOctWrap(n.xy);
	n.xy = n.xy * 0.5 + 0.5;
	return n.xy;
}

float3 NormalDecode(float2 f)
{
	f = f * 2.0 - 1.0;
	
	// https://twitter.com/Stubbesaurus/status/937994790553227264
	float3 n = float3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
	float t = saturate(-n.z);
	n.xy += float2(n.x >= 0.0 ? -t : t, n.y >= 0.0 ? -t : t);
	return normalize(n);
}

// Normals
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Random

namespace Random
{
	
	// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
	uint Hash(uint x)
	{
		x += (x << 10u);
		x ^= (x >> 6u);
		x += (x << 3u);
		x ^= (x >> 11u);
		x += (x << 15u);
		return x;
	}
	
	// Construct a float with half-open range [0:1] using low 23 bits.
	// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
	float FloatConstruct(uint m)
	{
		const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
		const uint ieeeOne = 0x3F800000u;      // 1.0 in IEEE binary32

		m &= ieeeMantissa; // Keep only mantissa bits (fractional part)
		m |= ieeeOne;      // Add fractional part to 1.0

		float f = asfloat(m); // Range [1:2]
		return f - 1.0;       // Range [0:1]
	}
	
	float rand(float x)
	{
		return FloatConstruct(Hash(asuint(x)));
	}
	
	
	// Steps the RNG and returns a floating-point value between 0 and 1 inclusive.
	float StepAndOutputRNGFloat(inout uint rngState)
	{
		// Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
		rngState = rngState * 747796405 + 1;
		uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
		word = (word >> 22) ^ word;
		return float(word) / 4294967295.0f;
	}
	
	// [0:1] quad
	float2 UniformDistrubition2d(inout uint rngState)
	{
		return float2(StepAndOutputRNGFloat(rngState), StepAndOutputRNGFloat(rngState));
	}
	
	float3 RandomDirectionHemisphere(float2 xi, float3 normal)
	{
	// For a random diffuse bounce direction, we follow the approach of
	// Ray Tracing in One Weekend, and generate a random point on a sphere
	// of radius 1 centered at the normal. This uses the random_unit_vector
	// function from chapter 8.5:
		float theta = 6.2831853 * xi.x; // Random in [0, 2pi]
		float u = 2.0 * xi.y - 1.0; // Random in [-1, 1]
		float r = sqrt(1.0 - u * u);
		float3 direction = normal + float3(r * cos(theta), r * sin(theta), u);
		return normalize(direction);
	}

	// Samples a direction within a hemisphere oriented along +Z axis with a cosine-weighted distribution 
	// Source: "Sampling Transformations Zoo" in Ray Tracing Gems by Shirley et al.
	// https://github.com/boksajak/brdf/blob/master/brdf.h
	float3 RandomDirectionHemisphereCosineLocal(float2 u, out float pdf)
	{
		u = clamp(u, 0.001, 0.999);

		float a = sqrt(u.x);
		float b = TWO_PI * u.y;

		float3 result = float3(a * cos(b), a * sin(b), sqrt(1.0f - u.x));

		pdf = result.z * ONE_OVER_PI;

		return result;
	}

	float3 RandomDirectionHemisphereCosine(float2 u, float3 normal, out float pdf)
	{
		float3 localdir = RandomDirectionHemisphereCosineLocal(u, pdf);
		return mul(localdir, ComputeTangentsFromVector(normal));
	}

	float3 RandomDirectionSphere(float2 xi)
	{
		float phi   = 6.2831853 * xi.x; // Random in [0, 2pi]
		float theta = 3.1415926 * xi.y; // Random in [0, 1pi]

		// spherical to cartesian
		return float3(sin(theta) * cos(phi),
					  sin(theta) * sin(phi),
					  cos(theta));
	}
	
	float3 RandomDirectionCone(float3 Direction, float BaseRadius, float2 Random)
	{
		// generate points in circle
		float theta   = Random.x * 2 * PI;
		float radius  = Random.y * 0.5 * BaseRadius;
		float2 circle = float2(cos(theta) * radius, sin(theta) * radius);

		// generate cone basis
		// TODO: verify handinness
		float3 up      = Direction.y < 0.999 ? float3(0, 1, 0) : float3(0, 0, 1);
		float3 right   = normalize(cross(up, Direction));
		float3 forward = normalize(cross(right, up));

		// use basis to transform points
		return Direction + circle.x * right + circle.y * forward;
	}

} // namespace Random

// Random
///////////////////////////////////////////////////////////////////
