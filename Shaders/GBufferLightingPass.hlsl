#include "BRDF.hlsli"

// we need only definitions from GPUScene
#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"

[[vk::binding(0, 0)]] Texture2D GBufferAlbedo;
[[vk::binding(1, 0)]] Texture2D GBufferNormal;
[[vk::binding(2, 0)]] Texture2D GBufferWorldPosition;
[[vk::binding(3, 0)]] Texture2D GBufferRoughnessMetallic;
[[vk::binding(4, 0)]] Texture2D GBufferLightmap; // TODO: unify with GI?
[[vk::binding(5, 0)]] Texture2D GBufferReflections;
[[vk::binding(6, 0)]] Texture2D GBufferGI;
[[vk::binding(7, 0)]] RWTexture2D<float4> LightingOutput;

[[vk::binding(8, 0)]] StructuredBuffer<GPULight> GPUSceneLights;
[[vk::binding(9, 0)]] StructuredBuffer<GPUSceneStruct> GPUSceneScene;

[[vk::binding(10, 0)]] Texture2D LTC_1;
[[vk::binding(11, 0)]] Texture2D LTC_2;
[[vk::binding(12, 0)]] SamplerState LTC_Sampler;

[[vk::binding(0, 1)]] RWTexture2D<float> ShadowTextures[1000];

static const float LTC_LUT_SIZE = 64.0;
static const float LTC_LUT_SCALE = (LTC_LUT_SIZE - 1.0) / LTC_LUT_SIZE;
static const float LTC_LUT_BIAS = 0.5 / LTC_LUT_SIZE;



// LTC CODE

// Vector form without project to the plane (dot with the normal)
// Use for proxy sphere clipping
float3 IntegrateEdgeVec(float3 v1, float3 v2)
{
	// Using built-in acos() function will result flaws
	// Using fitting result for calculating acos()
	float x = dot(v1, v2);
	float y = abs(x);

	float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
	float b = 3.4175940 + (4.1616724 + y) * y;
	float v = a / b;

	float theta_sintheta = (x > 0.0) ? v : 0.5 * rsqrt(max(1.0 - x * x, 1e-7)) - v;

	return cross(v1, v2) * theta_sintheta;
}

float3 LTC_Evaluate_Rect(float3 N, float3 V, float3 P, float3x3 Minv, float3 points[4], bool twoSided)
{
	// construct orthonormal basis around N
	float3 T1, T2;
	T1 = normalize(V - N * dot(V, N));
	T2 = cross(N, T1);

	// rotate area light in (T1, T2, N) basis
	Minv = mul(transpose(Minv), float3x3(T1, T2, N)); // me: rearranged some transpositions
	// Minv = mul(Minv, transpose(float3x3(N, T2, T1)) );

	// polygon (allocate 4 vertices for clipping)
	float3 L[4];
	// transform polygon from LTC back to origin Do (cosine weighted)
	L[0] = mul(Minv, (points[0] - P));
	L[1] = mul(Minv, (points[1] - P));
	L[2] = mul(Minv, (points[2] - P));
	L[3] = mul(Minv, (points[3] - P));

	// use tabulated horizon-clipped sphere
	// check if the shading point is behind the light
	float3 dir = points[0] - P; // LTC space
	float3 lightNormal = cross(points[1] - points[0], points[3] - points[0]);
	bool behind = (dot(dir, lightNormal) < 0.0);

	// cos weighted space
	L[0] = normalize(L[0]);
	L[1] = normalize(L[1]);
	L[2] = normalize(L[2]);
	L[3] = normalize(L[3]);

	// integrate
	float3 vsum = float3(0,0,0);
	vsum += IntegrateEdgeVec(L[0], L[1]);
	vsum += IntegrateEdgeVec(L[1], L[2]);
	vsum += IntegrateEdgeVec(L[2], L[3]);
	vsum += IntegrateEdgeVec(L[3], L[0]);

	// form factor of the polygon in direction vsum
	float len = length(vsum);

	float z = vsum.z / len;
	if (behind)
		z = -z;

	float2 uv = float2(z * 0.5f + 0.5f, len); // range [0, 1]
	uv = uv * LTC_LUT_SCALE + LTC_LUT_BIAS;

	// Fetch the form factor for horizon clipping
	float scale = LTC_2.SampleLevel(LTC_Sampler, uv, 0).w;

	float sum = len * scale;
	if (!behind && !twoSided)
		sum = 0.0;

	// Outgoing radiance (solid angle) for the entire polygon
	float3 Lo_i = float3(sum, sum, sum);
	return Lo_i;
}

// LTC CODE


// LTC DISC CODE

// An extended version of the implementation from
// "How to solve a cubic equation, revisited"
// http://momentsingraphics.de/?p=105
float3 SolveCubic(float4 Coefficient)
{
    // Normalize the polynomial
    Coefficient.xyz /= Coefficient.w;
    // Divide middle coefficients by three
    Coefficient.yz /= 3.0;

    float A = Coefficient.w;
    float B = Coefficient.z;
    float C = Coefficient.y;
    float D = Coefficient.x;

    // Compute the Hessian and the discriminant
    float3 Delta = float3(
        -Coefficient.z*Coefficient.z + Coefficient.y,
        -Coefficient.y*Coefficient.z + Coefficient.x,
        dot(float2(Coefficient.z, -Coefficient.y), Coefficient.xy)
    );

    float Discriminant = dot(float2(4.0*Delta.x, -Delta.y), Delta.zy);

    float3 RootsA, RootsD;

    float2 xlc, xsc;

    // Algorithm A
    {
        float A_a = 1.0;
        float C_a = Delta.x;
        float D_a = -2.0*B*Delta.x + Delta.y;

        // Take the cubic root of a normalized complex number
        float Theta = atan2(sqrt(Discriminant), -D_a)/3.0;

        float x_1a = 2.0*sqrt(-C_a)*cos(Theta);
        float x_3a = 2.0*sqrt(-C_a)*cos(Theta + (2.0/3.0)*PI);

        float xl;
        if ((x_1a + x_3a) > 2.0*B)
            xl = x_1a;
        else
            xl = x_3a;

        xlc = float2(xl - B, A);
    }

    // Algorithm D
    {
        float A_d = D;
        float C_d = Delta.z;
        float D_d = -D*Delta.y + 2.0*C*Delta.z;

        // Take the cubic root of a normalized complex number
        float Theta = atan2(D*sqrt(Discriminant), -D_d)/3.0;

        float x_1d = 2.0*sqrt(-C_d)*cos(Theta);
        float x_3d = 2.0*sqrt(-C_d)*cos(Theta + (2.0/3.0)*PI);

        float xs;
        if (x_1d + x_3d < 2.0*C)
            xs = x_1d;
        else
            xs = x_3d;

        xsc = float2(-D, xs + C);
    }

    float E =  xlc.y*xsc.y;
    float F = -xlc.x*xsc.y - xlc.y*xsc.x;
    float G =  xlc.x*xsc.x;

    float2 xmc = float2(C*F - B*G, -B*F + C*E);

    float3 Root = float3(xsc.x/xsc.y, xmc.x/xmc.y, xlc.x/xlc.y);

    if (Root.x < Root.y && Root.x < Root.z)
        Root.xyz = Root.yxz;
    else if (Root.z < Root.x && Root.z < Root.y)
        Root.xyz = Root.xzy;

    return Root;
}

float sqr(float x) { return x*x; }

float3 LTC_Evaluate_Disc(float3 N, float3 V, float3 P, float3x3 Minv, float3 points[4], bool twoSided)
{
    // construct orthonormal basis around N
    float3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

	Minv = transpose(Minv);

    // rotate area light in (T1, T2, N) basis
    float3x3 R = float3x3(T1, T2, N);
    // float3x3 R = transpose(float3x3(T1, T2, N));

    // polygon (allocate 5 vertices for clipping)
    float3 L_[3];
    L_[0] = mul(R, points[0] - P);
    L_[1] = mul(R, points[1] - P);
    L_[2] = mul(R, points[2] - P);

    float3 Lo_i = float3(0,0,0);

    // init ellipse
    float3 C  = 0.5 * (L_[0] + L_[2]);
    float3 V1 = 0.5 * (L_[1] - L_[2]);
    float3 V2 = 0.5 * (L_[1] - L_[0]);

    C  = mul(Minv, C);
    V1 = mul(Minv, V1);
    V2 = mul(Minv, V2);

    if(!twoSided && dot(cross(V1, V2), C) < 0.0)
        return float3(0,0,0);

    // compute eigenvectors of ellipse
    float a, b;
    float d11 = dot(V1, V1);
    float d22 = dot(V2, V2);
    float d12 = dot(V1, V2);
    if (abs(d12)/sqrt(d11*d22) > 0.0001)
    {
        float tr = d11 + d22;
        float det = -d12*d12 + d11*d22;

        // use sqrt matrix to solve for eigenvalues
        det = sqrt(det);
        float u = 0.5*sqrt(tr - 2.0*det);
        float v = 0.5*sqrt(tr + 2.0*det);
        float e_max = sqr(u + v);
        float e_min = sqr(u - v);

        float3 V1_, V2_;

        if (d11 > d22)
        {
            V1_ = d12*V1 + (e_max - d11)*V2;
            V2_ = d12*V1 + (e_min - d11)*V2;
        }
        else
        {
            V1_ = d12*V2 + (e_max - d22)*V1;
            V2_ = d12*V2 + (e_min - d22)*V1;
        }

        a = 1.0 / e_max;
        b = 1.0 / e_min;
        V1 = normalize(V1_);
        V2 = normalize(V2_);
    }
    else
    {
        a = 1.0 / dot(V1, V1);
        b = 1.0 / dot(V2, V2);
        V1 *= sqrt(a);
        V2 *= sqrt(b);
    }

    float3 V3 = cross(V1, V2);
    if (dot(C, V3) < 0.0)
        V3 *= -1.0;

    float L  = dot(V3, C);
    float x0 = dot(V1, C) / L;
    float y0 = dot(V2, C) / L;

    float E1 = rsqrt(a);
    float E2 = rsqrt(b);

    a *= L*L;
    b *= L*L;

    float c0 = a*b;
    float c1 = a*b*(1.0 + x0*x0 + y0*y0) - a - b;
    float c2 = 1.0 - a*(1.0 + x0*x0) - b*(1.0 + y0*y0);
    float c3 = 1.0;

    float3 roots = SolveCubic(float4(c0, c1, c2, c3));
    float e1 = roots.x;
    float e2 = roots.y;
    float e3 = roots.z;

    float3 avgDir = float3(a*x0/(a - e2), b*y0/(b - e2), 1.0);

    float3x3 rotate = float3x3(V1, V2, V3);

    avgDir = mul(rotate, avgDir);
    avgDir = normalize(avgDir);

    float L1 = sqrt(-e2/e3);
    float L2 = sqrt(-e2/e1);

    float formFactor = L1*L2*rsqrt((1.0 + L1*L1)*(1.0 + L2*L2));

    // use tabulated horizon-clipped sphere
    float2 uv = float2(avgDir.z*0.5 + 0.5, formFactor);
    uv = uv * LTC_LUT_SCALE + LTC_LUT_BIAS;
	float scale = LTC_2.SampleLevel(LTC_Sampler, uv, 0).w;

    float spec = formFactor*scale;

    Lo_i = float3(spec, spec, spec);

    return float3(Lo_i);
}

// LTC DISC CODE



[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	const uint2 Pixel = dtid.xy;

	if (any(Pixel >= GPUSceneScene[0].RenderSize))
		return;

	const float3 WP = GBufferWorldPosition[Pixel].rgb;
	const float2 RM = GBufferRoughnessMetallic[Pixel].rg;

	BRDFData BRDF;
	BRDF.N         = GBufferNormal[Pixel].rgb;
	BRDF.V         = -normalize(WP - GPUSceneScene[0].CameraCur.CameraPosition.xyz);
	BRDF.Albedo    = GBufferAlbedo[Pixel].rgb;
	BRDF.Roughness = RM.x;
	BRDF.Metallic  = RM.y;
	
	float MetalFactor = 1 - RM.y;
	float NdotV = saturate(dot(BRDF.N, BRDF.V));
	
	// LTC initialisation

	// use roughness and sqrt(1-cos_theta) to sample M_texture
	float2 LTC_uv = float2(BRDF.Roughness, sqrt(1.0f - NdotV));
	LTC_uv = LTC_uv * LTC_LUT_SCALE + LTC_LUT_BIAS;

	float4 t1 = LTC_1.SampleLevel(LTC_Sampler, LTC_uv, 0); // get 4 parameters for inverse_M
	float4 t2 = LTC_2.SampleLevel(LTC_Sampler, LTC_uv, 0); // Get 2 parameters for Fresnel calculation

	float3x3 Minv = float3x3(
		float3(t1.x, 0, t1.y),
		float3(0,    1,    0),
		float3(t1.z, 0, t1.w)
	);
	float3x3 LTC_identity = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);

	//float3 Lightmap = GBufferLightmap[Pixel].rgb;
	//float3 LightingSum = Lightmap * Albedo;
	float3 LightingSum = float3(0, 0, 0);

	// TODO: move lighting functions to a common header
	for (uint i = 0; i < GPUSceneScene[0].LightsCount; i++)
	{
		GPULight Light = GPUSceneLights[i];

		float Shadow = 1.0f;

		if ((Light.Flags & GPULIGHT_FLAG_SHADOW) != 0)
			Shadow = ShadowTextures[i][Pixel].r;

		float3 LightDir = Light.Direction.xyz;

		float Attenuation = 0;
		float Distance    = distance(WP, Light.Position.xyz);
		float LightRange  = Light.Range;
		
		float3 LightValue = float3(0, 0, 0);

		switch (Light.Type)
		{
		case GPULIGHT_DIRECTIONAL:
			Attenuation = 1; // directional light doesn't attenuate
			
			LightValue = max(dot(BRDF.N, LightDir), 0) * Shadow * Attenuation * Light.Color.rgb;
			BRDF.L = LightDir;
			LightingSum += EvaluateBRDF(BRDF, LightValue);
			break;
		case GPULIGHT_POINT:
			Attenuation  = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0);
			Attenuation *= Attenuation;
			LightDir     = -normalize(WP - Light.Position.xyz);
			
			LightValue = max(dot(BRDF.N, LightDir), 0) * Shadow * Attenuation * Light.Color.rgb;
			BRDF.L = LightDir;
			LightingSum += EvaluateBRDF(BRDF, LightValue);
			break;
		case GPULIGHT_SPOT:
		{
			Attenuation  = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0);
			Attenuation *= Attenuation;
			LightDir     = -normalize(WP - Light.Position.xyz);

			float angle = saturate(dot(LightDir, Light.Direction.xyz));
			float2 angles = Light.SizeOrSpotAngles;

			Attenuation *= smoothstep(angles.y, angles.x, angle);

			LightValue = max(dot(BRDF.N, LightDir), 0) * Shadow * Attenuation * Light.Color.rgb;
			BRDF.L = LightDir;
			LightingSum += EvaluateBRDF(BRDF, LightValue);
		}
		break;
		case GPULIGHT_RECTANGLE:
			{
				// TODO: make it more stable
				float3x3 LTC_Axis = ComputeTangentsFromVector(Light.Direction.xyz);

				float3 points[4];
				float2 halfSize = Light.SizeOrSpotAngles;
				float3 ex = LTC_Axis[0] * halfSize.x;
				float3 ey = LTC_Axis[1] * halfSize.y;

				points[0] = Light.Position.xyz - ex - ey;
				points[1] = Light.Position.xyz + ex - ey;
				points[2] = Light.Position.xyz + ex + ey;
				points[3] = Light.Position.xyz - ex + ey;

				bool twoSided = (Light.Flags & GPULIGHT_FLAG_TWOSIDED) != 0;
				
				float3 diffuse = LTC_Evaluate_Rect(BRDF.N, BRDF.V, WP, LTC_identity, points, twoSided) * BRDF.Albedo * MetalFactor;
                float3 specular = LTC_Evaluate_Rect(BRDF.N, BRDF.V, WP, Minv, points, twoSided);
				float3 specularColor = float3(0.4, 0.4, 0.4);
				specular *= specularColor*t2.x + (1.0 - specularColor)*t2.y;

				LightingSum += (diffuse + specular) * Light.Color.rgb * Shadow;
            }
			break;

		case GPULIGHT_DISC:
			{
				float3x3 LTC_Axis = ComputeTangentsFromVector(Light.Direction.xyz);

				float3 points[4];
				float2 halfSize = Light.SizeOrSpotAngles;
				float3 ex = LTC_Axis[0] * halfSize.x;
				float3 ey = LTC_Axis[1] * halfSize.y;

				points[0] = Light.Position.xyz - ex - ey;
				points[1] = Light.Position.xyz + ex - ey;
				points[2] = Light.Position.xyz + ex + ey;
				points[3] = Light.Position.xyz - ex + ey;

				bool twoSided = (Light.Flags & 1) != 0;

				float3 diffuse = LTC_Evaluate_Disc(BRDF.N, BRDF.V, WP, LTC_identity, points, twoSided) * BRDF.Albedo * MetalFactor;
                //float3 specular = LTC_Evaluate_Disc(BRDF.N, BRDF.V, WP, Minv, points, twoSided);
				float3 specularColor = float3(0.4, 0.4, 0.4);
				// specular *= specularColor*t2.x + (1.0 - specularColor)*t2.y;

				LightingSum += (diffuse) * Light.Color.rgb/* * Shadow*/;
			}
			break;
		default:
			break;
		}
	}

	// apply indirect
	{
		LightingSum += GBufferReflections[Pixel].rgb;
		LightingSum += BRDF.Albedo * GBufferGI[Pixel].rgb * MetalFactor;
	}
	
	// TEST
	//LightingSum = GBufferLightmap[Pixel].rgb;
	
	LightingOutput[Pixel] = float4(LightingSum, 1);
}