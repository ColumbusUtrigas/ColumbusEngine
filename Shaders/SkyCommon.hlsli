#pragma once

// TODO: common sky tracing function used in raster and raytracing passes
// TODO: sky spherical harmonic for fast lookup of sky irradiance

// TODO: make different sky types: colour, gradient, procedural, texture

#define PI 3.14159265359

// Dimensions
// TODO: make them tunable properties
#define PLANET_RADIUS     6371e3
#define ATMOSPHERE_HEIGHT 100e3
#define RAYLEIGH_HEIGHT   8e3
#define MIE_HEIGHT        1.2e3
#define OZONE_PEAK_LEVEL  30e3
#define OZONE_FALLOFF     3e3
// Scattering coefficients
// TODO: make them tunable properties
#define BETA_RAY   float3(3.8e-6, 13.5e-6, 33.1e-6) // float3(5.5e-6, 13.0e-6, 22.4e-6)
#define BETA_MIE   float3(21e-6, 21e-6, 21e-6)
#define BETA_OZONE float3(2.04e-5, 4.97e-5, 1.95e-6)
#define SKY_G          0.75
// Samples
#define SAMPLES          4
#define LIGHT_SAMPLES    1 // Set to more than 1 for a realistic, less vibrant sunset

// Other
// TODO: make them tunable properties
#define SUN_ILLUMINANCE   128000.0
#define MOON_ILLUMINANCE  0.32
#define SPACE_ILLUMINANCE 0.01

#define ATMOSPHERE_RADIUS (PLANET_RADIUS + ATMOSPHERE_HEIGHT)

///////////////////////////////////////////////////////////////////
// Nishita sky simulation functions
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky.html
// https://sebh.github.io/publications/egsr2020.pdf
// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/master/Resources/RenderSkyCommon.hlsl
// https://www.shadertoy.com/view/wllyW4
// https://www.shadertoy.com/view/slSXRW

namespace Sky
{

/**
 * Computes entry and exit points of ray intersecting a sphere.
 *
 * @param origin    ray origin
 * @param dir       normalized ray direction
 * @param radius    radius of the sphere
 *
 * @return    .x - position of entry point relative to the ray origin | .y - position of exit point relative to the ray origin | if there's no intersection at all, .x is larger than .y
 */
float2 RaySphereIntersect(in float3 origin, in float3 dir, in float radius)
{
	float a = dot(dir, dir);
	float b = 2.0 * dot(dir, origin);
	float c = dot(origin, origin) - (radius * radius);
	float d = (b * b) - 4.0 * a * c;
	
	if (d < 0.0)
		return float2(1.0, -1.0);

	return float2
	(
		(-b - sqrt(d)) / (2.0 * a),
		(-b + sqrt(d)) / (2.0 * a)
	);
}

/**
 * Phase function used for Rayleigh scattering.
 *
 * @param cosTheta    cosine of the angle between light vector and view direction
 *
 * @return    Rayleigh phase function value
 */
float PhaseRayleigh(in float cosTheta)
{
	return (3.0 * (1.0 + cosTheta * cosTheta)) / (16.0 * PI);
}

// Henyey-Greenstein phase function, used for Mie scattering.
// @param cosTheta    cosine of the angle between light vector and view direction
// @param g           scattering factor | -1 to 0 - backward | 0 - isotropic | 0 to 1 - forward
float3 PhaseMie(in float cosTheta, in float g)
{
	float gg = g * g;
	return (1.0 - gg) / (4.0 * PI * pow(1.0 + gg - 2.0 * g * cosTheta, 1.5));
}

// Approximates density values for a given point around the planet.
// @param pos    position of the point, for which densities are calculated
// @return    .x - Rayleigh density | .y - Mie density | .z - ozone density
float3 AvgDensities(in float3 pos)
{
	float height = length(pos) - PLANET_RADIUS; // Height above surface
	float3 density;
	density.x = exp(-height / RAYLEIGH_HEIGHT);
	density.y = exp(-height / MIE_HEIGHT);
	density.z = (1.0 / cosh((OZONE_PEAK_LEVEL - height) / OZONE_FALLOFF)) * density.x; // Ozone absorption scales with rayleigh
	return density;
}

/**
 * Calculates atmospheric scattering value for a ray intersecting the planet.
 *
 * @param pos         ray origin
 * @param dir         ray direction
 * @param lightDir    light vector
 *
 * @return    sky color
 */
float3 Atmosphere(
	in float3 pos,
	in float3 dir,
	in float3 lightDir
)
{
	pos += float3(0.0, PLANET_RADIUS + 2.0, 0.0);
		
	// Intersect the atmosphere
	float2 intersect = RaySphereIntersect(pos, dir, ATMOSPHERE_RADIUS);

	// Accumulators
	float3 opticalDepth = float3(0, 0, 0); // Accumulated density of particles participating in Rayleigh, Mie and ozone scattering respectively
	float3 sumR = float3(0, 0, 0);
	float3 sumM = float3(0, 0, 0);
	
	// Here's the trick - we clamp the sampling length to keep precision at the horizon
	// This introduces banding, but we can compensate for that by scaling the clamp according to horizon angle
	float rayPos = max(0.0, intersect.x);
	float maxLen = ATMOSPHERE_HEIGHT;
	maxLen *= (1.0 - abs(dir.y) * 0.5);
	float stepSize = min(intersect.y - rayPos, maxLen) / float(SAMPLES);
	rayPos += stepSize * 0.5; // Let's sample in the center
	
	for (int i = 0; i < SAMPLES; i++)
	{
		float3 samplePos = pos + dir * rayPos; // Current sampling position

		// Similar to the primary iteration
		float2 lightIntersect = RaySphereIntersect(samplePos, lightDir, ATMOSPHERE_RADIUS); // No need to check if intersection happened as we already are inside the sphere

		float3 lightOpticalDepth = float3(0, 0, 0);
		
		// We're inside the sphere now, hence we don't have to clamp ray pos
		float lightStep = lightIntersect.y / float(LIGHT_SAMPLES);
		float lightRayPos = lightStep * 0.5; // Let's sample in the center
		
		for (int j = 0; j < LIGHT_SAMPLES; j++)
		{
			float3 lightSamplePos = samplePos + lightDir * (lightRayPos);

			lightOpticalDepth += AvgDensities(lightSamplePos) * lightStep;

			lightRayPos += lightStep;
		}

		// Accumulate optical depth
		float3 densities = AvgDensities(samplePos) * stepSize;
		opticalDepth += densities;

		// Accumulate scattered light
		float3 scattered = exp(-(BETA_RAY * (opticalDepth.x + lightOpticalDepth.x) + BETA_MIE * (opticalDepth.y + lightOpticalDepth.y) + BETA_OZONE * (opticalDepth.z + lightOpticalDepth.z)));
		sumR += scattered * densities.x;
		sumM += scattered * densities.y;

		rayPos += stepSize;
	}

	float cosTheta = dot(dir, lightDir);
		
	float Illuminance = SUN_ILLUMINANCE;
	
	float Exposure = 16.0 / Illuminance;
	Exposure = min(Exposure, 16.0 / (MOON_ILLUMINANCE * 8.0)); // Clamp the exposure to make night appear darker

	return max(
		PhaseRayleigh(cosTheta) * BETA_RAY * sumR + // Rayleigh color
		PhaseMie(cosTheta, SKY_G) * BETA_MIE * sumM, // Mie color
		0.0
	) * Illuminance * Exposure;
}

} // namespace Sky