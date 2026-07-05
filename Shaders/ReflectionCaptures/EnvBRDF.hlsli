#pragma once

static const float ENV_BRDF_LUT_SIZE = 128.0;
static const float ENV_BRDF_LUT_SCALE = (ENV_BRDF_LUT_SIZE - 1.0) / ENV_BRDF_LUT_SIZE;
static const float ENV_BRDF_LUT_BIAS = 0.5 / ENV_BRDF_LUT_SIZE;

float3 SampleEnvBRDF(Texture2D<float2> LUT, SamplerState LUTSampler, float3 F0, float Roughness, float NoV)
{
	float2 UV = float2(saturate(Roughness), saturate(NoV)) * ENV_BRDF_LUT_SCALE + ENV_BRDF_LUT_BIAS;
	float2 ScaleBias = LUT.SampleLevel(LUTSampler, UV, 0.0).rg;
	return F0 * ScaleBias.x + ScaleBias.y;
}

float EnvPrefilteredMip(float Roughness, float MipCount)
{
	return saturate(Roughness) * (max(MipCount, 1.0) - 1.0);
}

float3 ApplyEnvBRDF(Texture2D<float2> LUT, SamplerState LUTSampler, float3 PrefilteredRadiance, float3 F0, float Roughness, float NoV)
{
	return PrefilteredRadiance * SampleEnvBRDF(LUT, LUTSampler, F0, Roughness, NoV);
}
