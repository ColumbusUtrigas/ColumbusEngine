#ifndef COMMON_GLSL
#define COMMON_GLSL

#define GOLDEN_RATIO 1.618033988749894
#define PI 3.14159265359
#define TWO_PI 6.28318530718
#define ONE_OVER_PI 0.318310
#define EPSILON 0.0001

#define saturate(x) clamp(x, 0, 1)
#define lerp(x, y, s) mix(x, y, s)

// Math

mat3 ComputeTangentsFromVector(vec3 normal)
{
	// Using right-hand coord
	const vec3 up = abs(normal.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
	const vec3 xAxis = normalize(cross(up, normal));
	const vec3 yAxis = cross(normal, xAxis);
	const vec3 zAxis = normal;

	return mat3(xAxis, yAxis, zAxis);
}

// Random

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash(uint x)
{
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m )
{
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

	m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	m |= ieeeOne;                          // Add fractional part to 1.0

	float  f = uintBitsToFloat( m );       // Range [1:2]
	return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float rand(float x) { return floatConstruct(hash(floatBitsToUint(x))); }

// Steps the RNG and returns a floating-point value between 0 and 1 inclusive.
float stepAndOutputRNGFloat(inout uint rngState)
{
	// Condensed version of pcg_output_rxs_m_xs_32_32, with simple conversion to floating-point [0,1].
	rngState  = rngState * 747796405 + 1;
	uint word = ((rngState >> ((rngState >> 28) + 4)) ^ rngState) * 277803737;
	word      = (word >> 22) ^ word;
	return float(word) / 4294967295.0f;
}

// [0:1] quad
vec2 UniformDistrubition2d(inout uint rngState)
{
    return vec2(stepAndOutputRNGFloat(rngState), stepAndOutputRNGFloat(rngState));
}

// Sampling

#ifdef GPUSCENE_GLSL
vec3 SampleConeRay(vec3 Direction, float BaseRadius, vec2 Random)
{
	// generate points in circle
	float theta = Random.x * 2 * PI;
	float radius = Random.y * 0.5 * BaseRadius;
	vec2 circle = vec2(cos(theta) * radius, sin(theta) * radius);

	// generate cone basis
	// TODO: verify handinness
	vec3 up = Direction.y < 0.999 ? vec3(0, 1, 0) : vec3(0, 0, 1);
	vec3 right = normalize(cross(up, Direction));
	vec3 forward = normalize(cross(right, up));

	// use basis to transform points
	return Direction + circle.x * right + circle.y * forward;
}

vec3 SampleDirectionalLight(GPULight Light, vec3 origin, vec3 normal, vec2 Random)
{
	vec3 direction = normalize(Light.Direction.xyz);
	direction = SampleConeRay(direction, Light.SourceRadius, Random);

	// sample shadow
	traceRayEXT(acc, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
		0xFF, 0, 0, 0, origin, 0, direction, 5000, 1);

	// was shadow ray occluded?
	if (shadowPayload.colorAndDist.w > 0)
	{
		return vec3(0);
	}
	else
	{
		return max(dot(normal, direction), 0) * Light.Color.rgb;
	}
}

vec3 SamplePointLight(GPULight Light, vec3 origin, vec3 normal, vec2 Random)
{
	vec3 direction = normalize(Light.Position.xyz - origin);
	float dist = distance(Light.Position.xyz, origin);

	float normalisedConeBase = Light.SourceRadius / dist;
	direction = SampleConeRay(direction, normalisedConeBase, Random);

	// float attenuation = 1.0 / (1.0 + dist);
	float attenuation = clamp(1.0 - dist * dist / (Light.Range * Light.Range), 0.0, 1.0);
	attenuation *= attenuation;

	// sample shadow
	traceRayEXT(acc, gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
		0xFF, 0, 0, 0, origin, 0, direction, dist, 1);

	// was shadow ray occluded?
	if (shadowPayload.colorAndDist.w > 0)
	{
		return vec3(0);
	}
	else
	{
		return max(dot(normal, direction), 0) * attenuation * Light.Color.rgb;
	}
}
#endif // GPUSCENE_GLSL

vec3 RandomDirectionHemisphere(vec2 xi, vec3 normal)
{
	// For a random diffuse bounce direction, we follow the approach of
	// Ray Tracing in One Weekend, and generate a random point on a sphere
	// of radius 1 centered at the normal. This uses the random_unit_vector
	// function from chapter 8.5:
	float theta    = 6.2831853 * xi.x;  // Random in [0, 2pi]
	float u        = 2.0 * xi.y - 1.0;  // Random in [-1, 1]
	float r        = sqrt(1.0 - u * u);
	vec3 direction = normal + vec3(r * cos(theta), r * sin(theta), u);
	return normalize(direction);
}

// Samples a direction within a hemisphere oriented along +Z axis with a cosine-weighted distribution 
// Source: "Sampling Transformations Zoo" in Ray Tracing Gems by Shirley et al.
// https://github.com/boksajak/brdf/blob/master/brdf.h
vec3 RandomDirectionHemisphereCosineLocal(vec2 u, out float pdf)
{
	u = clamp(u, 0.001, 0.999);

	float a = sqrt(u.x);
	float b = TWO_PI * u.y;

	vec3 result = vec3(a * cos(b), a * sin(b), sqrt(1.0f - u.x));

	pdf = result.z * ONE_OVER_PI;

	return result;
}

vec3 RandomDirectionHemisphereCosine(vec2 u, vec3 normal, out float pdf)
{
	vec3 localdir = RandomDirectionHemisphereCosineLocal(u, pdf);
	return ComputeTangentsFromVector(normal) * localdir;
}

vec3 RandomDirectionSphere(vec2 xi)
{
	float phi   = 6.2831853 * xi.x; // Random in [0, 2pi]
	float theta = 3.1415926 * xi.y; // Random in [0, 1pi]

	return vec3(sin(theta) * cos(phi),
	            sin(theta) * sin(phi),
	            cos(theta));
}

#if 0
// Importance sampling

// PDF = D * NoH / (4 * VoH)
// xyz - dir, w - PDF
vec4 RandomLocalDirecitonGGX(float roughness, vec2 xi)
{
	roughness = clamp(roughness, 0.01, 1);

	const float phi = 2.0 * PI * xi.x;
	const float a2 = roughness*roughness;

	xi.y = clamp(xi.y + 0.01, 0, 1);

	// Only near the specular direction according to the roughness for importance sampling
	const float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a2 - 1.0) * xi.y));
	const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	float d = (cosTheta * a2 - cosTheta) * cosTheta + 1;
	float D = a2 / (PI*d*d);
	float PDF = D * cosTheta;

	return vec4(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta, PDF);
}

vec4 RandomDirectionGGX(float roughness, vec3 normal, vec2 xi)
{
	const vec4 localSample = RandomLocalDirecitonGGX(roughness, xi);
	const vec3 localDir = localSample.xyz;
	const mat3 tanSpace = ComputeTangentsFromVector(normal);

	return vec4(tanSpace * localDir, localSample.w);
}
#endif

#endif // COMMON_GLSL