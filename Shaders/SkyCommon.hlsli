#pragma once

// TODO: common sky tracing function used in raster and raytracing passes
// TODO: sky spherical harmonic for fast lookup of sky irradiance

// TODO: make different sky types: colour, gradient, procedural, texture

#define PI 3.14159265359
#define SKY_G 0.75

/*
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
// Samples
#define SAMPLES          4
#define LIGHT_SAMPLES    1 // Set to more than 1 for a realistic, less vibrant sunset

// Other
// TODO: make them tunable properties
#define SUN_ILLUMINANCE   128000.0
#define MOON_ILLUMINANCE  0.32
#define SPACE_ILLUMINANCE 0.01
#define ATMOSPHERE_RADIUS (PLANET_RADIUS + ATMOSPHERE_HEIGHT)
*/

struct SkySettings
{
	// scaterring coefficients: w parameter unused for GPU alignments
	float4 BetaRayleigh;
	float4 BetaMie;
	float4 BetaOzone;

	// samples
	int Samples;
	int LightSamples; // Set to more than 1 for a realistic, less vibrant sunset

	// scattering distributions, dimensions, in meters
	float PlanetRadius;
	float AtmosphereHeight;
	float RayleightHeight;
	float MieHeight;
	float OzonePeak;
	float OzoneFalloff;

	// illuminance
	float SunIlluminance;
	float MoonIlluminance;
	float SpaceIlluminance;
	float Exposure;
	
	// sun disc
	float4 SunDiscColour;
	float  SunAngularRadiusRad;
	float  SunDiscSoftness;

	// stars
	float StarsBrightness;
	float StarsDensity;

	// 128
};

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
float3 AvgDensities(in float3 pos, in SkySettings Settings)
{
	float height = length(pos) - Settings.PlanetRadius; // Height above surface
	float3 density;
	density.x = exp(-height / Settings.RayleightHeight);
	density.y = exp(-height / Settings.MieHeight);
	density.z = (1.0 / cosh((Settings.OzonePeak - height) / Settings.OzoneFalloff)) * density.x; // Ozone absorption scales with rayleigh
	return density;
}

float3 SunDisc(float3 dir, float3 sunDir, in SkySettings Settings)
{
	float cosAngle = dot(dir, sunDir);
	float angle = acos(clamp(cosAngle, -1.0, 1.0)); // Angular distance
	
	// Obstruction logic, amount of disc visible
	float sunScreenY = dot(dir, float3(0, 1, 0)); // world up
	float obstruction = smoothstep(-Settings.SunDiscSoftness, Settings.SunDiscSoftness, sunScreenY);
		
	float intensity = smoothstep(Settings.SunAngularRadiusRad + Settings.SunDiscSoftness, Settings.SunAngularRadiusRad, angle);
	return Settings.SunDiscColour.rgb * intensity * obstruction;
}
	
float3 ProceduralStars(float3 dir, float3 sunDir, in SkySettings Settings)
{
	// === Generate a coherent "star cell" ===
	const float starDensity = Settings.StarsDensity;
	float3 cell = floor(dir * starDensity);
	float3 cellDir = normalize(cell / starDensity);

	// === Hash the cell to get unique but repeatable seed ===
	float h = frac(sin(dot(cell, float3(12.9898,78.233,45.164))) * 43758.5453);
	if (h < 0.995) return float3(0.0, 0.0, 0.0); // only some cells have stars

	// === Star properties ===
	float angularDist = acos(dot(dir, cellDir)); // angular distance to center of cell
	float radius = 0.003;                         // star disc radius in radians
	float softness = radius * 0.5;
	float disc = smoothstep(radius + softness, radius, angularDist);

	// Horizon obstruction
	float sunScreenY = dot(dir, float3(0, 1, 0)); // world up
	float obstruction = smoothstep(-Settings.SunDiscSoftness, Settings.SunDiscSoftness, sunScreenY);

	// === Color variation ===
	float hue = frac(sin(dot(cell, float3(95.435, 12.783, 46.971))) * 145837.253);

	// Temperature-based color
	float3 starColor = float3(
		saturate(1.5 - 6.0 * abs(hue - 0.0)),   // Red to White
		saturate(1.5 - 6.0 * abs(hue - 0.33)),  // Orange/Yellow to White
		saturate(1.5 - 6.0 * abs(hue - 0.66))   // Blue to White
	);
	starColor = lerp(starColor, float3(1,1,1), 0.5); // adjust star variation

	float brightness = (h - 0.995) * starDensity * disc; // brighter for rare stars

	return float3(brightness, brightness, brightness) * obstruction * starColor * Settings.StarsBrightness;
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
	in float3 lightDir,
	in SkySettings Settings
)
{
	float AtmosphereRadius = Settings.PlanetRadius + Settings.AtmosphereHeight;
	pos += float3(0.0, Settings.PlanetRadius + 2.0, 0.0);
		
	// Intersect the atmosphere
	float2 intersect = RaySphereIntersect(pos, dir, AtmosphereRadius);

	// Accumulators
	float3 opticalDepth = float3(0, 0, 0); // Accumulated density of particles participating in Rayleigh, Mie and ozone scattering respectively
	float3 sumR = float3(0, 0, 0);
	float3 sumM = float3(0, 0, 0);
	
	// Here's the trick - we clamp the sampling length to keep precision at the horizon
	// This introduces banding, but we can compensate for that by scaling the clamp according to horizon angle
	float rayPos = max(0.0, intersect.x);
	float maxLen = Settings.AtmosphereHeight;
	maxLen *= (1.0 - abs(dir.y) * 0.5);
	float stepSize = min(intersect.y - rayPos, maxLen) / float(Settings.Samples);
	rayPos += stepSize * 0.5; // Let's sample in the center
	
	for (int i = 0; i < Settings.Samples; i++)
	{
		float3 samplePos = pos + dir * rayPos; // Current sampling position

		// Similar to the primary iteration
		float2 lightIntersect = RaySphereIntersect(samplePos, lightDir, AtmosphereRadius); // No need to check if intersection happened as we already are inside the sphere

		float3 lightOpticalDepth = float3(0, 0, 0);
		
		// We're inside the sphere now, hence we don't have to clamp ray pos
		float lightStep = lightIntersect.y / float(Settings.LightSamples);
		float lightRayPos = lightStep * 0.5; // Let's sample in the center
		
		for (int j = 0; j < Settings.LightSamples; j++)
		{
			float3 lightSamplePos = samplePos + lightDir * (lightRayPos);

			lightOpticalDepth += AvgDensities(lightSamplePos, Settings) * lightStep;

			lightRayPos += lightStep;
		}

		// Accumulate optical depth
		float3 densities = AvgDensities(samplePos, Settings) * stepSize;
		opticalDepth += densities;

		// Accumulate scattered light
		float3 scattered = exp(-(Settings.BetaRayleigh.xyz * (opticalDepth.x + lightOpticalDepth.x) + Settings.BetaMie.xyz * (opticalDepth.y + lightOpticalDepth.y) + Settings.BetaOzone.xyz * (opticalDepth.z + lightOpticalDepth.z)));
		sumR += scattered * densities.x;
		sumM += scattered * densities.y;

		rayPos += stepSize;
	}

	float cosTheta = dot(dir, lightDir);
		
	float Illuminance = Settings.SunIlluminance;
	
	float Exposure = Settings.Exposure / Illuminance;
	Exposure = min(Exposure, Settings.Exposure / (Settings.MoonIlluminance * Settings.Exposure*0.5f)); // Clamp the exposure to make night appear darker

	float3 skyColour = max(
		PhaseRayleigh(cosTheta) * Settings.BetaRayleigh.xyz * sumR + // Rayleigh color
		PhaseMie(cosTheta, SKY_G) * Settings.BetaMie.xyz * sumM, // Mie color
		0.0
	) * Illuminance * Exposure;

	skyColour += SunDisc(dir, lightDir, Settings);

	if (Settings.StarsBrightness > 0.01f)
	{
		skyColour += ProceduralStars(dir, lightDir, Settings);
	}
		
	return skyColour;
}

} // namespace Sky