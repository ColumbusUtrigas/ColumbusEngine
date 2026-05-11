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

#define USE_EON_DIFFUSE_BRDF 1

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

float LambertDiffusePDF(float3 Normal, float3 L)
{
	return saturate(dot(Normal, L)) * ONE_OVER_PI;
}

// EON diffuse BRDF
// from "EON: A Practical Energy-Preserving Rough Diffuse BRDF" 2025 by Portsmouth, Kutz, Hill

static const float RCP_PI = 1.0f / PI;
static const float CONSTANT_1_FON = 0.5f - 2.0f / (3.0f * PI);
static const float CONSTANT_2_FON = 2.0f / 3.0f - 28.0f / (15.0f * PI);

float E_FON_Exact(float mu, float r)
{
    float AF = 1.0f / (1.0f + CONSTANT_1_FON * r); // FON A coefficient
    float BF = r * AF; // FON B coefficient

    float Si = sqrt(1.0f - mu * mu);
    float G = Si * (acos(mu) - Si * mu)
             + (2.0f / 3.0f) * ((Si / mu) * (1.0f - Si * Si * Si) - Si);

    return AF + BF * RCP_PI * G;
}

float E_FON_Approx(float mu, float r)
{
    float mucomp = 1.0f - mu;

    static const float g1 = 0.057108529f;
    static const float g2 = 0.491881867f;
    static const float g3 = -0.332181442f;
    static const float g4 = 0.0714429953f;

    float GoverPi = mucomp * (g1 + mucomp * (g2 + mucomp * (g3 + mucomp * g4)));

    return (1.0f + r * GoverPi) / (1.0f + CONSTANT_1_FON * r);
}

// Evaluates EON BRDF value, given inputs:
// rho = single-scattering albedo parameter
// r   = roughness in [0, 1]
// exact = flag to select exact or fast approximate version
//
// Assumes directions are in local space where z aligns with the surface normal.
float3 F_EON(float3 rho, float r, float3 wi_local, float3 wo_local, bool exact)
{
    float mu_i = wi_local.z; // input angle cos
    float mu_o = wo_local.z; // output angle cos

    float s = dot(wi_local, wo_local) - mu_i * mu_o; // QON s term
    float sovert = (s > 0.0f) ? s / max(mu_i, mu_o) : s; // FON s/t

    float AF = 1.0f / (1.0f + CONSTANT_1_FON * r); // FON A coefficient

    float3 f_ss = rho * RCP_PI * AF * (1.0f + r * sovert); // single-scatter lobe

    float EFo = exact ? E_FON_Exact(mu_o, r) : E_FON_Approx(mu_o, r); // FON wo albedo
    float EFi = exact ? E_FON_Exact(mu_i, r) : E_FON_Approx(mu_i, r); // FON wi albedo

    float avgEF = AF * (1.0f + CONSTANT_2_FON * r); // average albedo

    float3 rho_ms = (rho * rho) * avgEF / (float3(1.0f, 1.0f, 1.0f) - rho * (1.0f - avgEF));

    static const float eps = 1.0e-7f;

    float3 f_ms = (rho_ms * RCP_PI)
                * max(eps, 1.0f - EFo)
                * max(eps, 1.0f - EFi)
                / max(eps, 1.0f - avgEF);

    return f_ss + f_ms;
}

// Computes EON directional albedo.
float3 E_EON(float3 rho, float r, float3 wi_local, bool exact)
{
    float mu_i = wi_local.z; // input angle cos
    float AF = 1.0f / (1.0f + CONSTANT_1_FON * r); // FON A coefficient
    float EF = exact ? E_FON_Exact(mu_i, r) : E_FON_Approx(mu_i, r); // FON wi albedo
    float avgEF = AF * (1.0f + CONSTANT_2_FON * r); // average albedo
    float3 rho_ms = (rho * rho) * avgEF / (float3(1.0f, 1.0f, 1.0f) - rho * (1.0f - avgEF));
    return rho * EF + rho_ms * (1.0f - EF);
}

void EON_LTC_Coeffs(float mu, float r, out float a, out float b, out float c, out float d)
{
    a = 1.0f + r * (0.303392f + (-0.518982f + 0.111709f * mu) * mu + (-0.276266f + 0.335918f * mu) * r);
    b = r * (-1.16407f + 1.15859f * mu + (0.150815f - 0.150105f * mu) * r) / (mu * mu * mu - 1.43545f);
    c = 1.0f + r * (0.20013f + (-0.506373f + 0.261777f * mu) * mu);
    d = r * (0.540852f + (-1.01625f + 0.475392f * mu) * mu) / (-1.0743f + (0.0725628f + mu) * mu);
}

float3x3 OrthonormalBasisLTC(float3 wo_local)
{
	float3 X = float3(wo_local.x, wo_local.y, 0.0f);
	if (dot(X, X) < 1.0e-5f)
	{
		X = float3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		X = normalize(X);
	}

	float3 Y = float3(-X.y, X.x, 0.0f);
	float3 Z = float3(0.0f, 0.0f, 1.0f);

	return float3x3(X, Y, Z);
}

float3 EON_UniformLobeSample(float u1, float u2)
{
	float SinTheta = sqrt(max(1.0f - u1 * u1, 0.0f));
	float Phi = TWO_PI * u2;
	return float3(SinTheta * cos(Phi), SinTheta * sin(Phi), u1);
}

float4 EON_LTC_Sample(float3 wo_local, float r, float u1, float u2)
{
    float a, b, c, d;
    EON_LTC_Coeffs(wo_local.z, r, a, b, c, d); // Coeffs of LTC M

    // CLTC sampling
    float R = sqrt(u1);
    float phi = 2.0f * PI * u2;

    float x = R * cos(phi);
    float y = R * sin(phi);

    // CLTC sampling factors
    float vz = 1.0f / sqrt(d * d + 1.0f);
    float s = 0.5f * (1.0f + vz);

    x = -lerp(sqrt(1.0f - y * y), x, s);

    // omega_H sample via CLTC
    float3 wh = float3(
        x,
        y,
        sqrt(max(1.0f - (x * x + y * y), 0.0f))
    );

    // PDF of omega_H sample
    float pdf_wh = wh.z / (PI * s);

    // M * omega_H, unnormalized
    float3 wi = float3(
        a * wh.x + b * wh.z,
        c * wh.y,
        d * wh.x + wh.z
    );

    float len = max(length(wi), 1.0e-7f);

    // |M|
    float detM = max(c * (a - b * d), 1.0e-7f);

    // omega_i sample PDF
    float pdf_wi = pdf_wh * len * len * len / detM;

    // omega_i -> local space
    float3x3 fromLTC = ComputeTangentsFromVector(wo_local);
    wi = normalize(mul(wi, fromLTC));

    return float4(wi, pdf_wi);
}

float EON_LTC_PDF(float3 wo_local, float3 wi_local, float r)
{
    // omega_i -> LTC space
    float3x3 toLTC = ComputeTangentsFromVector(wo_local);
    float3 wi = mul(toLTC, wi_local);

    float a, b, c, d;
    EON_LTC_Coeffs(wo_local.z, r, a, b, c, d); // Coeffs of LTC M

    // |M|
    float detM = max(c * (a - b * d), 1.0e-7f);

    // adj(M) * omega_i
    float3 wh = float3(
        c * (wi.x - b * wi.z),
        (a - b * d) * wi.y,
        -c * (d * wi.x - a * wi.z)
    );

    // ||M||M^-1 omega_i||
    float lenSqr = max(dot(wh, wh), 1.0e-7f);

    // CLTC sampling factors
    float vz = 1.0f / sqrt(d * d + 1.0f);
    float s = 0.5f * (1.0f + vz);

    // omega_i sample PDF
    float pdf = detM * detM / (lenSqr * lenSqr)
              * max(wh.z, 0.0f)
              / (PI * s);

    return pdf;
}

float EON_UniformLobeProbability(float3 wo_local, float r)
{
	float mu = saturate(wo_local.z);
	float Pu = pow(saturate(r), 0.1f) * (0.162925f + (-0.372058f + (0.538233f - 0.290822f * mu) * mu) * mu);
	return saturate(Pu);
}

float4 EON_Sample(float3 wo_local, float r, float u1, float u2)
{
	float Pu = EON_UniformLobeProbability(wo_local, r);
	float Pc = 1.0f - Pu;

	float3 wi;
	float pdf_c;
	if (u1 <= Pu && Pu > 1.0e-5f)
	{
		u1 /= Pu;
		wi = EON_UniformLobeSample(u1, u2);
		pdf_c = EON_LTC_PDF(wo_local, wi, r);
	}
	else
	{
		u1 = Pc > 1.0e-5f ? (u1 - Pu) / Pc : u1;
		float4 CltcSample = EON_LTC_Sample(wo_local, r, u1, u2);
		wi = CltcSample.xyz;
		pdf_c = CltcSample.w;
	}

	float pdf_u = 0.5f * ONE_OVER_PI;
	float pdf = Pu * pdf_u + Pc * pdf_c;
	return float4(wi, max(pdf, 1.0e-7f));
}

float EON_PDF(float3 wo_local, float3 wi_local, float r)
{
	float Pu = EON_UniformLobeProbability(wo_local, r);
	float Pc = 1.0f - Pu;
	float pdf_u = 0.5f * ONE_OVER_PI;
	float pdf_c = EON_LTC_PDF(wo_local, wi_local, r);
	return max(Pu * pdf_u + Pc * pdf_c, 1.0e-7f);
}

///////////////////////////////////////////////////////////////////////////////

BRDFSample SampleDiffuseBRDF(BRDFData Data, float2 Xi)
{
#if USE_EON_DIFFUSE_BRDF
	float3x3 LocalTransform = ComputeTangentsFromVector(Data.N);
	float3 WoLocal = mul(LocalTransform, Data.V);
	WoLocal.z = max(WoLocal.z, 1.0e-4f);

	float4 EonSample = EON_Sample(normalize(WoLocal), saturate(Data.Roughness), Xi.x, Xi.y);

	BRDFSample Sample;
	Sample.Dir = normalize(mul(EonSample.xyz, LocalTransform));
	Sample.Pdf = EonSample.w;
	return Sample;
#else
	return SampleBRDF_Lambert(Data.N, Xi);
#endif
}

float DiffuseBRDFPDF(BRDFData Data, float3 L)
{
#if USE_EON_DIFFUSE_BRDF
	float3x3 LocalTransform = ComputeTangentsFromVector(Data.N);
	float3 WiLocal = mul(LocalTransform, L);
	float3 WoLocal = mul(LocalTransform, Data.V);
	WiLocal.z = max(WiLocal.z, 1.0e-4f);
	WoLocal.z = max(WoLocal.z, 1.0e-4f);
	return EON_PDF(normalize(WoLocal), normalize(WiLocal), saturate(Data.Roughness));
#else
	return LambertDiffusePDF(Data.N, L);
#endif
}

float3 EvaluateDiffuseBRDF(BRDFData Data)
{
	float NdotL = saturate(dot(Data.N, Data.L));
	float NdotV = saturate(dot(Data.N, Data.V));
	if (NdotL <= 0.0f || NdotV <= 0.0f)
	{
		return 0.0.xxx;
	}

#if USE_EON_DIFFUSE_BRDF
	float3x3 LocalTransform = ComputeTangentsFromVector(Data.N);
	float3 WiLocal = mul(LocalTransform, Data.L);
	float3 WoLocal = mul(LocalTransform, Data.V);
	WiLocal.z = max(WiLocal.z, 1.0e-4f);
	WoLocal.z = max(WoLocal.z, 1.0e-4f);

	return F_EON(saturate(Data.Albedo), saturate(Data.Roughness), normalize(WiLocal), normalize(WoLocal), false);
#else
	return LambertDiffuseBRDF(Data.Albedo);
#endif
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

float3 BRDFF0(BRDFData Data)
{
    return lerp(float3(0.04, 0.04, 0.04), Data.Albedo, Data.Metallic);
}

float3 FresnelSchlick(float CosTheta, float3 F0)
{
    return F0 + (1.0.xxx - F0) * pow(1.0 - saturate(CosTheta), 5.0);
}

float3 EvaluateSpecularBRDF(BRDFData Data)
{
	float3 H = normalize(Data.V + Data.L);

	float NdotV = saturate(dot(Data.N, Data.V));
	float NdotL = saturate(dot(Data.N, Data.L));
	float VdotH = saturate(dot(Data.V, H));

	if (NdotV <= 0.0 || NdotL <= 0.0)
		return 0.0.xxx;

	float Roughness = max(Data.Roughness, 0.02);
	float3 F = FresnelSchlick(VdotH, BRDFF0(Data));
	float D = DistributionGGX(Data.N, H, Roughness);
	float Vis = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

	return D * F * Vis;
}

float3 EvaluateBRDF(BRDFData Data)
{
	float3 H = normalize(Data.V + Data.L);
	float NdotV = saturate(dot(Data.N, Data.V));
	float NdotL = saturate(dot(Data.N, Data.L));
	float VdotH = saturate(dot(Data.V, H));

	if (NdotV <= 0.0 || NdotL <= 0.0)
		return 0.0.xxx;

	float3 F = FresnelSchlick(VdotH, BRDFF0(Data));
	float3 DiffuseFactor = (1.0.xxx - F) * (1.0 - Data.Metallic);

	return DiffuseFactor * EvaluateDiffuseBRDF(Data) + EvaluateSpecularBRDF(Data);
}

float3 EvaluateBRDFCos(BRDFData Data)
{
	return EvaluateBRDF(Data) * saturate(dot(Data.N, Data.L));
}

float DiffusePDF(BRDFData Data, float3 L)
{
	return DiffuseBRDFPDF(Data, L);
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

// Calculates (SpecularBRDF * NdotL) / PDF without evaluating D (Distribution).
// This cancels out the unstable terms at low roughness.
// Formula derived from: (D * F * Vis * NdotL) / (D * NdotH / (4 * VdotH))
float3 EvaluateReflectionBRDF(BRDFData Data)
{
    float3 H = normalize(Data.V + Data.L);
    float3 N = Data.N;
    float3 V = Data.V;
    float3 L = Data.L;
    float Roughness = Data.Roughness;

    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));

	// FRESNEL
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, Data.Albedo, Data.Metallic);
    float3 F = FresnelSchlick(VdotH, F0);

	// GEOMETRY (VISIBILITY)
	// GeometryGGX function actually calculates the Visibility term (G / (4 * NdotL * NdotV))
    float Vis = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

	// COMBINE
	// The D term cancels out
	// We are left with: F * Vis * NdotL * (4 * VdotH / NdotH)
	
	// Safety to prevent division by zero
    if (NdotH <= 0.0001)
        return float3(0, 0, 0);

    return F * Vis * NdotL * (4.0 * VdotH / NdotH);
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
	float VdotH = max(dot(V, H), 0.0);

	float D = DistributionGGX(N, H, Roughness);

	return D * NdotH / (4.0 * VdotH);
}
