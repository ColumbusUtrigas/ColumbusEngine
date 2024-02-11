#ifndef BRDF_GLSL
#define BRDF_GLSL

#define GOLDEN_RATIO 1.618033988749894
#define PI 3.14159265359

struct BRDFData
{
	vec3 N; // normal
	vec3 V; // view
	vec3 L; // light direction

	vec3 Albedo;
	float Roughness;
	float Metallic;
};

vec3 LambertDiffuseBRDF(vec3 x)
{
	return x * 0.318310; // 1.0 / PI
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometryGGX(float a, float ndotl, float ndotv)
{
	float a2 = max(a * a, 0.001f);

	float gv = ndotl * sqrt(a2 + (1.0f - a2) * ndotv * ndotv);
	float gl = ndotv * sqrt(a2 + (1.0f - a2) * ndotl * ndotl);

	return 0.5f / max(gv + gl, 0.001f);
}

// TODO: function with F0 and F90
vec3 Fresnel(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// TODO: function with F0 and F90
vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CookTorranceSpecularBRDF(vec3 N, vec3 L, vec3 H, vec3 F, float NdotV, float Roughness)
{
	float NdotL = max(0.0, dot(N, L));

	float D = DistributionGGX(N, H, Roughness);
	float G = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

	return LambertDiffuseBRDF(D * F * G) * NdotL;
}

vec3 EvaluateBRDF(BRDFData Data, vec3 Sample)
{
	vec3 N = Data.N;
	vec3 V = Data.V;
	vec3 L = Data.L;
	vec3 R = reflect(-V, N);
	float Roughness = Data.Roughness;
	float Metallic = Data.Metallic;

	vec3 F0 = vec3(0.04); // TODO: expose F0/F90 into material
	F0 = mix(F0, vec3(1), Metallic);

	float NdotV = max(0.0, dot(N, V));
	float NdotL = max(0.0, dot(N, L));

	vec3 F = FresnelRoughness(NdotV, F0, Roughness);
	vec3 H = normalize(V + L);

	// vec3 DiffuseBRDF = LambertDiffuseBRDF(Data.Albedo) * clamp(dot(L, N), 0, 1); // * AO
	// vec3 DiffuseBRDF = LambertDiffuseBRDF(vec3(1)) * clamp(dot(L, N), 0, 1); // * AO
	vec3 DiffuseBRDF = LambertDiffuseBRDF(vec3(1)); // * AO
	vec3 SpecularBRDF = CookTorranceSpecularBRDF(N, L, H, F, NdotV, Roughness);
	float DiffuseFactor = 1.0 - Metallic;

	return (DiffuseFactor * DiffuseBRDF + SpecularBRDF) * Sample;
}

#endif // BRDF_GLSL