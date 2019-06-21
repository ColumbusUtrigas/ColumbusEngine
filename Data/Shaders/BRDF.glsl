#define LAMBERTIAN 0.318310 // 1.0 / PI

vec3 LambertDiffuseBRDF(vec3 color)
{
	return color * LAMBERTIAN;
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

vec3 Fresnel(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CookTorranceSpecularBRDF(vec3 N, vec3 L, vec3 H, vec3 F, float NdotV, float Roughness)
{
	float NdotL = max(0.0, dot(N, L));

	float D = DistributionGGX(N, H, Roughness);
	float G = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

	return D * F * G * LAMBERTIAN * NdotL;
}


