#include "../BRDF.hlsli"
#include "ReflectionCaptureBakeCommon.hlsli"

[[vk::binding(0, 0)]] TextureCube<float4> SourceRadiance;
[[vk::binding(1, 0)]] SamplerState SourceSampler;
[[vk::binding(2, 0)]] RWTexture2DArray<float4> TargetMip;

// mirrored in Graphics/RenderPasses/ReflectionCaptures.cpp
[[vk::push_constant]]
struct _Params
{
	int TargetMipSize;
	int TargetMip;
	int MipCount;
	int SampleOffset;

	int SamplesThisPass;
	int TotalSamples;
	int _Pad0;
	int _Pad1;
} Params;

// GGX-convolves mip 0 of the capture cubemap into one lower mip.
// dispatched as (TargetMipSize, 6 * TargetMipSize) threads; y selects the cube face.
[numthreads(8, 8, 1)]
void main(uint3 DispatchThreadId : SV_DispatchThreadID)
{
	uint MipSize = (uint)max(Params.TargetMipSize, 1);
	uint2 Pixel = DispatchThreadId.xy;
	uint Face = Pixel.y / MipSize;
	if (Pixel.x >= MipSize || Face >= 6u)
		return;

	uint2 FacePixel = uint2(Pixel.x, Pixel.y - Face * MipSize);
	float3 Direction = ReflectionCaptureCubeFaceDirection(Face, FacePixel, MipSize);

	uint TotalSamples = (uint)max(Params.TotalSamples, 1);
	uint SampleOffset = (uint)Params.SampleOffset;
	if (SampleOffset >= TotalSamples)
		return;

	uint SamplesThisPass = min((uint)max(Params.SamplesThisPass, 1), TotalSamples - SampleOffset);
	bool IsFirstSampleChunk = SampleOffset == 0;
	bool IsLastSampleChunk = SampleOffset + SamplesThisPass >= TotalSamples;

	uint MipCount = (uint)max(Params.MipCount, 1);
	float Roughness = MipCount <= 1u ? 0.0 : float(Params.TargetMip) / float(MipCount - 1u);

	BRDFData BRDF;
	BRDF.N = Direction;
	BRDF.V = Direction;
	BRDF.L = Direction;
	BRDF.Albedo = 1.0.xxx;
	BRDF.Roughness = Roughness;
	BRDF.Metallic = 1.0;

	// sample chunks accumulate in place; weight lives in .a until the last chunk normalizes
	uint3 Texel = uint3(FacePixel, Face);
	float4 Accumulated = IsFirstSampleChunk ? float4(0, 0, 0, 0) : TargetMip[Texel];
	float3 RadianceSum = Accumulated.rgb;
	float WeightSum = Accumulated.a;

	uint TexelIndex = Face * MipSize * MipSize + FacePixel.y * MipSize + FacePixel.x;

	[loop]
	for (uint LocalSample = 0; LocalSample < SamplesThisPass; LocalSample++)
	{
		uint SampleIndex = SampleOffset + LocalSample;
		BRDFSample Sample = SampleBRDF_GGX(BRDF, ReflectionCaptureRotatedSample(TexelIndex, SampleIndex, TotalSamples));
		float3 SampleDirection = normalize(Sample.Dir);
		float NoL = saturate(dot(Direction, SampleDirection));
		if (NoL <= 0.0)
			continue;

		RadianceSum += SourceRadiance.SampleLevel(SourceSampler, SampleDirection, 0).rgb * NoL;
		WeightSum += NoL;
	}

	if (IsLastSampleChunk)
	{
		RadianceSum /= max(WeightSum, 0.0001);
		WeightSum = 1.0;
	}

	TargetMip[Texel] = float4(RadianceSum, WeightSum);
}
