// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
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

vec3 SampleDirectionalLight(GPULight Light, vec3 origin, vec3 normal)
{
	vec3 direction = normalize(Light.Direction.xyz);

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

vec3 SamplePointLight(GPULight Light, vec3 origin, vec3 normal)
{
	vec3 direction = normalize(Light.Position.xyz - origin);
	float dist = distance(Light.Position.xyz, origin);
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

vec3 RandomDirectionHemisphere(inout uint rngState, vec3 normal)
{
	// For a random diffuse bounce direction, we follow the approach of
	// Ray Tracing in One Weekend, and generate a random point on a sphere
	// of radius 1 centered at the normal. This uses the random_unit_vector
	// function from chapter 8.5:
	float theta    = 6.2831853 * stepAndOutputRNGFloat(rngState);  // Random in [0, 2pi]
	float u        = 2.0 * stepAndOutputRNGFloat(rngState) - 1.0;  // Random in [-1, 1]
	float r        = sqrt(1.0 - u * u);
	vec3 direction = normal + vec3(r * cos(theta), r * sin(theta), u);
	return normalize(direction);
}

vec3 RandomDirectionSphere(inout uint rngState)
{
	float phi   = 6.2831853 * stepAndOutputRNGFloat(rngState); // Random in [0, 2pi]
	float theta = 3.1415926 * stepAndOutputRNGFloat(rngState); // Random in [0, 1pi]

	return vec3(sin(theta) * cos(phi),
	            sin(theta) * sin(phi),
	            cos(theta));
}

// TODO
vec3 calculateLight(inout vec3 origin, vec3 direction, out vec3 normal, out vec3 surfaceColor, out int hitSurface, int lightsCount)
{
	// TODO
	traceRayEXT(acc, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, origin, 0, direction, 5000, 0);

	// if we hit a surface
	if (payload.colorAndDist.w > 0)
	{
		hitSurface = 1;

		surfaceColor = payload.colorAndDist.rgb;
		vec3 hitPoint = payload.colorAndDist.w * direction + origin;
		normal = payload.normalAndObjId.xyz;
		origin = hitPoint + normal * 0.001;

		vec3 AccumulatedLight = vec3(0);

		for (uint i = 0; i < lightsCount; i++)
		{
			GPULight Light = GPUSceneLights.Lights[i];
			switch (Light.Type)
			{
			case GPULIGHT_DIRECTIONAL:
				AccumulatedLight += SampleDirectionalLight(Light, origin, normal);
				break;
			case GPULIGHT_POINT:
				AccumulatedLight += SamplePointLight(Light, origin, normal);
				break;
			}
		}

		return AccumulatedLight * surfaceColor;
	}
	else // sky
	{
		hitSurface = 0;
		return payload.colorAndDist.rgb;
	}
}

vec3 PathTrace(vec3 Origin, vec3 Direction, int MaxBounces, inout uint RngState, int LightsCount)
{
	// First ray is always calculated
	int HitSurface = 0;
	vec3 SurfaceColor = vec3(0);
	vec3 Normal = vec3(0);
	vec3 Color = calculateLight(Origin, Direction, Normal, SurfaceColor, HitSurface, LightsCount);

	vec3 BounceColor = vec3(0);
	for (int i = 0; i < MaxBounces; i++)
	{
		vec3 BounceSurfaceColor = vec3(0);
		Direction = RandomDirectionHemisphere(RngState, Normal);

		BounceColor += calculateLight(Origin, Direction, Normal, BounceSurfaceColor, HitSurface, LightsCount);
		if (HitSurface == 0) break;
	}

	return Color + BounceColor * SurfaceColor;
}
