#pragma once

#include "Common.hlsli"

// references
// [0] https://boksajak.github.io/files/CrashCourseBRDF.pdf
// https://github.com/boksajak/brdf/blob/master/brdf.h
// https://agraphicsguynotes.com/posts/sample_microfacet_brdf/
// https://www.reedbeta.com/blog/hows-the-ndf-really-defined/
// https://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// https://computergraphics.stackexchange.com/questions/4979/what-is-importance-sampling

struct BRDFData
{
	float3 N; // normal
	float3 V; // view, dir_to_camera
	float3 L; // light direction

	float3 Albedo;
	float  Roughness;
	float  Metallic;
};

struct BRDFSample
{
	float3 Dir;
	float  Pdf;
};

float3 LambertDiffuseBRDF(float3 Albedo)
{
	return Albedo * ONE_OVER_PI;
}

BRDFSample SampleBRDF_Lambert(float3 Normal, float2 Xi)
{
	float pdf;

	BRDFSample Sample;
	// uniform hemisphere, pdf = 1
	// Sample.Dir = RandomDirectionHemisphere(Xi, Normal);
	// Sample.Pdf = 1;

	// cosine weighted
	Sample.Dir = Random::RandomDirectionHemisphereCosine(Xi, Normal, pdf);
	Sample.Pdf = pdf;
	return Sample;
}

// D_GGX
// https://boksajak.github.io/files/CrashCourseBRDF.pdf
float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
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
// The function should be F_Schlick = F0 + (F90 - F0) * (1 - NdotV^5)
// https://boksajak.github.io/files/CrashCourseBRDF.pdf, section 4.3
float3 FresnelRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(1.0 - roughness, F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 CookTorranceSpecularBRDF(float3 N, float3 L, float3 H, float3 F, float NdotV, float Roughness)
{
	Roughness = max(Roughness, 0.01);

	float NdotL = max(0.0, dot(N, L));

	float D = DistributionGGX(N, H, Roughness);
	float G = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

	return D * F * G * NdotL;
}

// https://github.com/boksajak/brdf/blob/master/brdf.h#L758
// Samples a microfacet normal for the GGX distribution using VNDF method.
// Source: "Sampling the GGX Distribution of Visible Normals" by Heitz
// Source: "Sampling Visible GGX Normals with Spherical Caps" by Dupuy & Benyoub
// Random variables 'u' must be in <0;1) interval
// PDF is 'G1(NdotV) * D'
// Vlocal is a View direction transformed into local space
float3 VDNF_GGX(float3 Vlocal, float alpha, float2 u)
{
	// Source: "Sampling the GGX Distribution of Visible Normals" by Heitz
	// See also https://hal.inria.fr/hal-00996995v1/document and http://jcgt.org/published/0007/04/01/

	// Section 3.2: transforming the view direction to the hemisphere configuration
	float3 Vh = normalize(float3(alpha * Vlocal.x, alpha * Vlocal.y, Vlocal.z));

	// Section 4.1: orthonormal basis (with special case if cross product is zero)
	float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
	float3 T1 = lensq > 0.0f ? (float3(-Vh.y, Vh.x, 0.0f) / sqrt(lensq)) : float3(1.0f, 0.0f, 0.0f);
	float3 T2 = cross(Vh, T1);

	// Section 4.2: parameterization of the projected area
	float r = sqrt(u.x);
	float phi = TWO_PI * u.y;
	float t1 = r * cos(phi);
	float t2 = r * sin(phi);
	float s = 0.5f * (1.0f + Vh.z);
	t2 = lerp(sqrt(1.0f - t1 * t1), t2, s);

	// Section 4.3: reprojection onto hemisphere
	float3 Nh = t1 * T1 + t2 * T2 + sqrt(max(0.0f, 1.0f - t1 * t1 - t2 * t2)) * Vh;

	// Section 3.4: transforming the normal back to the ellipsoid configuration
	return normalize(float3(alpha * Nh.x, alpha * Nh.y, max(0.0f, Nh.z)));
}

// https://github.com/boksajak/brdf/blob/master/brdf.h#L807
// PDF of sampling a reflection vector L using 'VDNF_GGX'.
// Note that PDF of sampling given microfacet normal is (G1 * D) when vectors are in local space (in the hemisphere around shading normal). 
// Remaining terms (1.0f / (4.0f * NdotV)) are specific for reflection case, and come from multiplying PDF by jacobian of reflection operator
// float VDNF_GGX_PDF(float alpha, float alphaSquared, float NdotH, float NdotV, float LdotH)
float VDNF_GGX_PDF(float alpha, float3 V, float3 N, float3 H, float3 L)
{
	float NdotH = max(0.00001f, dot(N, H));
	float NdotV = max(0.00001f, dot(N, V));
	float NdotL = max(0.00001f, dot(N, L));
	float a2    = max(0.00001f, alpha * alpha);

	return DistributionGGX(N, H, alpha) * GeometryGGX(alpha, NdotL, NdotV);
	// return DistributionGGX(N, H, alpha) * GeometryGGX(alpha, NdotL, NdotV) / (4 * NdotV);
	// return (GGX_D(max(0.00001f, alphaSquared), NdotH) * Smith_G1_GGX(alpha, NdotV, alphaSquared, NdotV * NdotV)) / (4.0f * NdotV);
}

float3 ComputeLocalDirectionGGX(float a, float2 xi)
{
	a = max(0.0001, a);
	const float phi = 2.0 * PI * xi.x;

	// Only near the specular direction according to the roughness for importance sampling
	const float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	return float3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

// TODO: remove Sample from here
float3 EvaluateBRDF(BRDFData Data, float3 Sample)
{
	float3 N = Data.N;
	float3 V = Data.V;
	float3 L = Data.L;
	float Roughness = Data.Roughness;
	float Metallic = Data.Metallic;

	float3 F0 = float3(0.04, 0.04, 0.04); // TODO: expose F0/F90 into material
	F0 = lerp(F0, Data.Albedo, Metallic); // [0], section 4.3.1

	float NdotV = max(0.0, dot(N, V));
	float NdotL = max(0.0, dot(N, L));

	float3 F = FresnelRoughness(NdotV, F0, Roughness);
	float3 H = normalize(V + L);

	float3 DiffuseBRDF = LambertDiffuseBRDF(Data.Albedo); // * AO
	float3 SpecularBRDF = CookTorranceSpecularBRDF(N, L, H, F, NdotV, Roughness);
	float DiffuseFactor = 1.0 - Metallic;

	return (DiffuseFactor * DiffuseBRDF + SpecularBRDF) * Sample;
}

#define USE_VNDF_GGX 0

BRDFSample SampleBRDF_GGX(BRDFData Data, float2 Xi)
{
	float3x3 LocalTransform = ComputeTangentsFromVector(Data.N);

	float3 Hlocal; // microfacet normal (H) in local space
    float3 Vlocal = mul(LocalTransform, Data.V); // world to local

	//Data.Roughness = max(Data.Roughness, 0.001);
	
	if (Data.Roughness <= 0.01)
	{
		Hlocal = float3(0, 0, 1);
	}
	else
	{
#if USE_VNDF_GGX
		Hlocal = VDNF_GGX(Vlocal, Data.Roughness, Xi);
#else
		Hlocal = ComputeLocalDirectionGGX(Data.Roughness * Data.Roughness, Xi);
#endif
	}

	// reflect incident ray from the microfacet
	float3 Llocal = reflect(-Vlocal, Hlocal);

#if USE_VNDF_GGX
	// PDF is 'G1(NdotV) * D'
	float3 Nlocal = Data.N * LocalTransform;
	float Pdf = VDNF_GGX_PDF(Data.Roughness, Vlocal, Nlocal, Hlocal, Llocal);
	// float Pdf = DistributionGGX(Nlocal, Hlocal, Data.Roughness);
#else
	// Pdf = D * NoH / (4 * VoH);
    float3 Nlocal = mul(LocalTransform, Data.N);
	float NdotH   = max(0.01, dot(Nlocal, Hlocal));
	float VdotH   = max(0.01, dot(Vlocal, Hlocal));
	float Pdf     = DistributionGGX(Nlocal, Hlocal, Data.Roughness) * NdotH / (4 * VdotH);
#endif

	BRDFSample Sample;
    Sample.Dir = mul(Llocal, LocalTransform); // local to world
	Sample.Pdf = Pdf;
	// Sample.Dir = Hlocal;
	return Sample;
}

// TODO: EvaluateIndirectBRDF()

float BRDFCalcPDF(BRDFData Data)
{
	float3 N = Data.N;
	float3 V = Data.V;
	float3 L = Data.L;
	float3 H = normalize(V + L);
	float Roughness = Data.Roughness;

	float NdotH = max(dot(N, H), 0.0);
	float VdotH = max(dot(N, H), 0.0);

	float D = DistributionGGX(N, H, Roughness);

	return D * NdotH / (4.0 * VdotH);
}