[[vk::binding(0, 0)]]   Texture2D<float4> Image;
[[vk::binding(1, 0)]]   Texture2D<float4> History;
[[vk::binding(2, 0)]]   Texture2D<float2> VelocityImage;
[[vk::binding(3, 0)]] RWTexture2D<float4> Output;
[[vk::binding(4, 0)]]   SamplerState      LinearSampler;

// references:
// https://www.elopezr.com/temporal-aa-and-the-quest-for-the-holy-trail/

[[vk::push_constant]]
struct _Params {
	uint2 Resolution;
} Params;

float3 TonemapReinhard(float3 x)
{
	return x / (x + 1);
}

float3 TonemapInverseReinhard(float3 x)
{
	return x / (1 - x);
}

// got from https://github.com/PanosK92/SpartanEngine/blob/master/data/shaders/temporal_antialiasing.hlsl
float4 sample_catmull_rom(Texture2D tex_history, float2 uv, float2 texture_size)
{
    // based on: https://gist.github.com/TheRealMJP/c83b8c0f46b63f3a88a5986f4fa982b1

    // scale UV coordinates by the texture size to work in texel space
    float2 sample_position = uv * texture_size;

    // calculate the center of the starting texel in a 4x4 grid. The grid's [1, 1] position.
    float2 texPos1 = floor(sample_position - 0.5f) + 0.5f;

    // calculate the fractional part of the original sample position relative to the starting texel.
    float2 fractional_offset = sample_position - texPos1;

    // calculate the Catmull-Rom weights for each axis using the fractional offset.
    float2 w0 = fractional_offset * (-0.5f + fractional_offset * (1.0f - 0.5f * fractional_offset));
    float2 w1 = 1.0f + fractional_offset * fractional_offset * (-2.5f + 1.5f * fractional_offset);
    float2 w2 = fractional_offset * (0.5f + fractional_offset * (2.0f - 1.5f * fractional_offset));
    float2 w3 = fractional_offset * fractional_offset * (-0.5f + 0.5f * fractional_offset);

    // combine weights for the middle two samples and calculate their combined offset.
    float2 combinedWeightMiddleSamples = w1 + w2;
    float2 combinedOffsetMiddleSamples = w2 / combinedWeightMiddleSamples;

    // calculate final UV coordinates for texture sampling.
    float2 texPos0 = (texPos1 - 1) / texture_size;
    float2 texPos3 = (texPos1 + 2) / texture_size;
    float2 texPosMiddle = (texPos1 + combinedOffsetMiddleSamples) / texture_size;

    // sample the texture at the calculated UV coordinates and accumulate the results.
    float4 result = 0.0f;
    result += tex_history.SampleLevel(LinearSampler, float2(texPos0.x, texPos0.y), 0.0f) * w0.x * w0.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPosMiddle.x, texPos0.y), 0.0f) * combinedWeightMiddleSamples.x * w0.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPos3.x, texPos0.y), 0.0f) * w3.x * w0.y;

    result += tex_history.SampleLevel(LinearSampler, float2(texPos0.x, texPosMiddle.y), 0.0f) * w0.x * combinedWeightMiddleSamples.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPosMiddle.x, texPosMiddle.y), 0.0f) * combinedWeightMiddleSamples.x * combinedWeightMiddleSamples.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPos3.x, texPosMiddle.y), 0.0f) * w3.x * combinedWeightMiddleSamples.y;

    result += tex_history.SampleLevel(LinearSampler, float2(texPos0.x, texPos3.y), 0.0f) * w0.x * w3.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPosMiddle.x, texPos3.y), 0.0f) * combinedWeightMiddleSamples.x * w3.y;
    result += tex_history.SampleLevel(LinearSampler, float2(texPos3.x, texPos3.y), 0.0f) * w3.x * w3.y;

    return result;
}

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
	if (any(dtid.xy >= Params.Resolution))
		return;

    float2 Velocity = VelocityImage[dtid.xy] / 2;

	float2 Uv = float2(dtid.xy + 0.5f) / Params.Resolution;
    float2 UvReprojected = Uv - Velocity;

	float3 Current = Image.Load(int3(dtid.xy, 0)).rgb;
    float3 Previous = sample_catmull_rom(History, UvReprojected, Params.Resolution).rgb;

	// colour clamping
	{
		float3 MinColour = 9999.0, MaxColour = -9999.0;
		// Sample a 3x3 neighborhood to create a box in color space
		for (int x = -1; x <= 1; ++x)
		{
			for (int y = -1; y <= 1; ++y)
			{
				float3 color = Image[dtid.xy + int2(x, y)].rgb; // Sample neighbor
				MinColour = min(MinColour, color); // Take min and max
				MaxColour = max(MaxColour, color);
			}
		}
		Previous = clamp(Previous, MinColour, MaxColour);
	}

    float BlendFactor = 0.1;
    {
        float FactorScreen = (any(UvReprojected > 1) || any(UvReprojected < 0)) ? 1 : 0;
        float FactorDisocclusion = 0; // TODO:

        BlendFactor = saturate(BlendFactor + FactorScreen);
    }

	{
		Current = TonemapReinhard(Current);
		Previous = TonemapReinhard(Previous);

		Output[dtid.xy] = float4(TonemapInverseReinhard(lerp(Previous, Current, BlendFactor)), 1);
	}
}