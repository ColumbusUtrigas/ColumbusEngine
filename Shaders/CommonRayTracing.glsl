#include "BRDF.glsl"
#include "Common.glsl"

// TODO
vec3 calculateLight(inout vec3 origin, vec3 direction, out vec3 normal, out vec3 surfaceColor, out int hitSurface, inout uint rngState)
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

		BRDFData BRDF;
		BRDF.N = normalize(normal);
		BRDF.V = -direction;
		BRDF.Albedo = surfaceColor;
		BRDF.Roughness = payload.RoughnessMetallic.x;
		BRDF.Metallic = payload.RoughnessMetallic.y;

		// next event estimation
		for (uint i = 0; i < GPUScene_GetLightsCount(); i++)
		{
			vec3 LightSample = vec3(0);

			// TODO: get it from sample functions
			BRDF.L = vec3(0); // light direction

			vec2 Xi = UniformDistrubition2d(rngState);

			GPULight Light = GPUSceneLights.Lights[i];
			switch (Light.Type)
			{
			case GPULIGHT_DIRECTIONAL:
				LightSample = SampleDirectionalLight(Light, origin, normal, Xi);
				BRDF.L = Light.Direction.xyz;
				break;
			case GPULIGHT_POINT:
				LightSample = SamplePointLight(Light, origin, normal, Xi);
				// TODO: account for sphere light
				BRDF.L = normalize(Light.Position.xyz - origin);
				break;
			}

			AccumulatedLight += EvaluateBRDF(BRDF, LightSample);
		}
		
		// TODO: account for ray losing energy?

		// return AccumulatedLight * surfaceColor;
		return AccumulatedLight;
	}
	else // sky
	{
		hitSurface = 0;
		return payload.colorAndDist.rgb;
	}
}

vec3 PathTrace(vec3 Origin, vec3 Direction, int MaxBounces, inout uint RngState)
{
	// First ray is always calculated
	int HitSurface = 0;
	vec3 SurfaceColor = vec3(0);
	vec3 Normal = vec3(0);
	vec3 Color = calculateLight(Origin, Direction, Normal, SurfaceColor, HitSurface, RngState);

	vec3 BounceColor = vec3(0);
	for (int i = 0; i < MaxBounces; i++)
	{
		BRDFData BRDF;
		BRDF.N = normalize(Normal);
		BRDF.V = -Direction;
		BRDF.Albedo = SurfaceColor;
		BRDF.Roughness = payload.RoughnessMetallic.x;
		BRDF.Metallic = payload.RoughnessMetallic.y;

		vec3 BounceSurfaceColor = vec3(0);
		// Direction = RandomDirectionHemisphere(RngState, Normal);
		Direction = reflect(Direction, Normal);
		Direction = RandomDirectionGGX(BRDF.Roughness*BRDF.Roughness, Direction, UniformDistrubition2d(RngState));
		BRDF.L = Direction;

		vec3 Sample = calculateLight(Origin, Direction, Normal, BounceSurfaceColor, HitSurface, RngState);
		BounceColor += EvaluateBRDF(BRDF, Sample);

		if (HitSurface == 0) break;
	}

	return Color + BounceColor * SurfaceColor;
}
