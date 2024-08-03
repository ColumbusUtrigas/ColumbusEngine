#include "BRDF.glsl"
#include "Common.glsl"

#if 0
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
#endif

vec3 PathTrace(vec3 Origin, vec3 Direction, int MaxBounces, inout uint RngState)
{
	vec3 PathRadiance = vec3(0);
	vec3 PathAttenuation = vec3(1);
	BRDFData BRDF;

	// always at least one ray
	for (int i = 0; i <= MaxBounces; i++)
	{
		traceRayEXT(acc, gl_RayFlagsOpaqueEXT, 0xFF, 0, 0, 0, Origin, 0, Direction, 5000, 0);

		if (payload.colorAndDist.w > 0)
		{
			BRDF.Albedo = payload.colorAndDist.rgb;
			BRDF.N = payload.normalAndObjId.xyz;
			BRDF.V = -Direction;
			BRDF.Roughness = payload.RoughnessMetallic.x;
			BRDF.Metallic = payload.RoughnessMetallic.y;

			vec3 HitPoint = payload.colorAndDist.w * Direction + Origin;
			Origin = HitPoint + BRDF.N * 0.001;

			// next event estimation
			for (uint l = 0; l < GPUScene_GetLightsCount(); l++)
			{
				vec3 LightSample = vec3(0);

				// TODO: get it from sample functions
				BRDF.L = vec3(0); // light direction

				vec2 Xi = UniformDistrubition2d(RngState);
				// TODO: light PDF, as every time we do a random decision, we need to weight it by it's probability

				GPULight Light = GPUSceneLights.Lights[l];
				switch (Light.Type)
				{
				case GPULIGHT_DIRECTIONAL:
					LightSample = SampleDirectionalLight(Light, Origin, BRDF.N, Xi);
					BRDF.L = Light.Direction.xyz;
					break;
				case GPULIGHT_POINT:
					LightSample = SamplePointLight(Light, Origin, BRDF.N, Xi);
					// TODO: account for sphere light
					BRDF.L = normalize(Light.Position.xyz - Origin);
					break;
				}

				PathRadiance += EvaluateBRDF(BRDF, vec3(1)) * LightSample * PathAttenuation;
			}

			// diffuse only
			#if 1
			{
				BRDFSample Sample = SampleBRDF_GGX(BRDF, UniformDistrubition2d(RngState));
				// BRDFSample Sample = SampleBRDF_Lambert(BRDF.N, UniformDistrubition2d(RngState));
				BRDF.L = Sample.Dir;
				Direction = Sample.Dir;

				float NdotL = max(0, dot(BRDF.N, BRDF.L));

				PathAttenuation *= EvaluateBRDF(BRDF, vec3(1)) * NdotL / Sample.Pdf;

				// PathAttenuation *= LambertDiffuseBRDF(BRDF.Albedo) * NdotL / Sample.Pdf;
			}
			#endif

			#if 0
			// generate new ray for the next trace
			// Direction = reflect(Direction, BRDF.N); // TODO: reflect from H?
			vec3 R = reflect(Direction, BRDF.N); // TODO: reflect from H?
			Direction = RandomDirectionHemisphere(UniformDistrubition2d(RngState), BRDF.N);

			//vec3 H = normalize(-Direction + R);
			//float D = DistributionGGX(BRDF.N, H, BRDF.Roughness);
			//return vec3(D);

			//const vec4 ImportanceSample = RandomDirectionGGX(BRDF.Roughness*BRDF.Roughness, BRDF.V, UniformDistrubition2d(RngState));
			//Direction = ImportanceSample.xyz;
			//float PDF = ImportanceSample.w;

			// Direction = RandomDirectionGGX(BRDF.Roughness*BRDF.Roughness, Direction, UniformDistrubition2d(RngState));
			BRDF.L = Direction;

			PathAttenuation *= EvaluateBRDF(BRDF, vec3(1));
			#endif
		}
		else // sky
		{
			// blender default sky is 0.051 gray
			// PathRadiance += vec3(0.051) * PathAttenuation;

			PathRadiance += payload.colorAndDist.rgb * PathAttenuation;
			break;
		}
	}

	return PathRadiance;
}

#if 0
vec3 PathTraceOld(vec3 Origin, vec3 Direction, int MaxBounces, inout uint RngState)
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
#endif