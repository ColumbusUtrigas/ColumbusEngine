#ifndef COLUMBUS_SHADOW_MAPS
#define COLUMBUS_SHADOW_MAPS

// Defaults are the deferred lighting set. Other lighting paths can bind the same resources.
#ifndef SHADOW_MAP_SET
#define SHADOW_MAP_SET 3
#endif

struct GPUShadowView
{
	float4x4 WorldToClip;
	float4   AtlasScaleBias;
	float4   SampleBounds;
	float4   Cascade;
	uint     AtlasIndex;
	uint3    Padding;
};

[[vk::binding(0, SHADOW_MAP_SET)]] StructuredBuffer<GPUShadowView> ShadowViews;
[[vk::binding(1, SHADOW_MAP_SET)]] SamplerComparisonState          ShadowComparisonSampler;
[[vk::binding(2, SHADOW_MAP_SET)]] Texture2D<float>                ShadowAtlases[64];

uint ShadowPointFace(float3 FromLight)
{
	float3 A = abs(FromLight);

	if (A.x >= A.y && A.x >= A.z)
	{
		return FromLight.x >= 0 ? 0 : 1;
	}

	if (A.y >= A.z)
	{
		return FromLight.y >= 0 ? 2 : 3;
	}

	return FromLight.z >= 0 ? 4 : 5;
}

float SampleShadowView(GPULight Light, GPUShadowView View, float3 WorldPosition, float3 Normal)
{
	if (View.AtlasIndex == 0xffffffffu)
	{
		return 1;
	}

	float4 ClipPosition = mul(View.WorldToClip, float4(WorldPosition + Normal * Light.ShadowNormalBias, 1));

	if (ClipPosition.w <= 0)
	{
		return 1;
	}

	float3 Ndc = ClipPosition.xyz / ClipPosition.w;

	if (any(abs(Ndc.xy) > 1) || Ndc.z < 0 || Ndc.z > 1)
	{
		return 1;
	}

	float2 Uv = NDCToScreenUV(Ndc.xy) * View.AtlasScaleBias.xy + View.AtlasScaleBias.zw;
	uint   Width;
	uint   Height;
	uint   Atlas = NonUniformResourceIndex(View.AtlasIndex);
	ShadowAtlases[Atlas].GetDimensions(Width, Height);
	float2 Texel          = rcp(float2(Width, Height));
	float  ReferenceDepth = min(Ndc.z + Light.ShadowDepthBias, 1.0);
	float  Radius         = clamp(Light.ShadowFilterRadius, 0.0, 3.0);

	if (Radius == 0)
	{
		return ShadowAtlases[Atlas].SampleCmpLevelZero(
		    ShadowComparisonSampler, clamp(Uv, View.SampleBounds.xy, View.SampleBounds.zw), ReferenceDepth);
	}

	float Visibility = 0;

	[unroll]
	for (int Y = -1; Y <= 1; ++Y)
	{
		[unroll]
		for (int X = -1; X <= 1; ++X)
		{
			float2 SampleUV = clamp(Uv + float2(X, Y) * Radius * Texel, View.SampleBounds.xy, View.SampleBounds.zw);
			Visibility += ShadowAtlases[Atlas].SampleCmpLevelZero(ShadowComparisonSampler, SampleUV, ReferenceDepth);
		}
	}

	return Visibility / 9;
}

float EvaluateRasterShadow(GPULight Light, float3 WorldPosition, float3 Normal, float CameraDepth)
{
	if ((Light.Flags & GPULIGHT_FLAG_SHADOW) == 0 || (Light.Flags & GPULIGHT_FLAG_RAYTRACED_SHADOW) != 0 ||
	    Light.ShadowViewCount == 0)
	{
		return 1;
	}

	uint Selected = 0;

	if (Light.Type == GPULIGHT_DIRECTIONAL)
	{
		[loop]
		while (
		    Selected < Light.ShadowViewCount && CameraDepth > ShadowViews[Light.FirstShadowView + Selected].Cascade.y)
		{
			++Selected;
		}

		if (Selected >= Light.ShadowViewCount)
		{
			return 1;
		}
	}
	else if (Light.Type == GPULIGHT_POINT)
	{
		Selected = ShadowPointFace(WorldPosition + Normal * Light.ShadowNormalBias - Light.Position.xyz);
	}
	else if ((Light.Type == GPULIGHT_RECTANGLE || Light.Type == GPULIGHT_DISC) && Light.ShadowViewCount > 1)
	{
		Selected = dot(WorldPosition - Light.Position.xyz, Light.Direction.xyz) >= 0 ? 0 : 1;
	}

	GPUShadowView View   = ShadowViews[Light.FirstShadowView + Selected];
	float         Result = SampleShadowView(Light, View, WorldPosition, Normal);

	if (Light.Type == GPULIGHT_DIRECTIONAL)
	{
		float Blend = saturate((CameraDepth - View.Cascade.z) / max(View.Cascade.y - View.Cascade.z, 1e-5));

		if (Blend > 0)
		{
			float Next =
			    Selected + 1 < Light.ShadowViewCount
			        ? SampleShadowView(Light, ShadowViews[Light.FirstShadowView + Selected + 1], WorldPosition, Normal)
			        : 1;
			Result = lerp(Result, Next, Blend);
		}
	}
	else if (Light.Type == GPULIGHT_RECTANGLE || Light.Type == GPULIGHT_DISC)
	{
		// A finite shadow cone has finite coverage; fade its edge instead of cutting abruptly.
		float4 ClipPosition   = mul(View.WorldToClip, float4(WorldPosition, 1));
		float  Edge           = max(abs(ClipPosition.x), abs(ClipPosition.y)) / max(ClipPosition.w, 1e-5);
		Result                = lerp(Result, 1, smoothstep(0.9, 1.0, Edge));
		float DistanceToLight = distance(WorldPosition, Light.Position.xyz);
		Result                = lerp(Result, 1, smoothstep(Light.ShadowDistance * 0.9, Light.ShadowDistance, DistanceToLight));
	}
	return Result;
}
#endif
