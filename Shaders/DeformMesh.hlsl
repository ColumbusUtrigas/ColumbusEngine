#pragma pack_matrix(row_major)

[[vk::push_constant]]
struct _Params
{
	uint64_t BaseVertices;
	uint64_t BaseNormals;
	uint64_t BaseTangents;
	uint64_t BoneIndices;
	uint64_t BoneWeights;
	uint64_t SkinMatrices;
	uint64_t OutVertices;
	uint64_t OutPrevVertices;
	uint64_t OutNormals;
	uint64_t OutTangents;
	uint64_t MorphDeltas;
	uint64_t MorphWeights;
	uint VertexCount;
	uint SkinMatrixCount;
	uint MorphWeightCount;
	uint _pad;
} Params;

float3 LoadFloat3(uint64_t Address, uint Index)
{
	return vk::RawBufferLoad<float3>(Address + Index * 12);
}

float4 LoadFloat4(uint64_t Address, uint Index)
{
	return vk::RawBufferLoad<float4>(Address + Index * 16);
}

float LoadFloat(uint64_t Address, uint Index)
{
	return vk::RawBufferLoad<float>(Address + Index * 4);
}

int4 LoadInt4(uint64_t Address, uint Index)
{
	return vk::RawBufferLoad<int4>(Address + Index * 16);
}

float4x4 LoadMatrix(uint64_t Address, uint Index)
{
	uint64_t MatrixAddress = Address + Index * 64;
	float4x4 Result = (float4x4)0;
	Result[0] = vk::RawBufferLoad<float4>(MatrixAddress + 0);
	Result[1] = vk::RawBufferLoad<float4>(MatrixAddress + 16);
	Result[2] = vk::RawBufferLoad<float4>(MatrixAddress + 32);
	Result[3] = vk::RawBufferLoad<float4>(MatrixAddress + 48);
	return Result;
}

void StoreFloat3(uint64_t Address, uint Index, float3 Value)
{
	vk::RawBufferStore(Address + Index * 12, Value);
}

void StoreFloat4(uint64_t Address, uint Index, float4 Value)
{
	vk::RawBufferStore(Address + Index * 16, Value);
}

void AccumulateSkin(
	int JointIndex,
	float Weight,
	float3 Position,
	float3 Normal,
	float3 Tangent,
	inout float4 SkinnedPosition,
	inout float3 SkinnedNormal,
	inout float3 SkinnedTangent,
	inout float AppliedWeight)
{
	if (Weight <= 0.0f || JointIndex < 0 || JointIndex >= (int)Params.SkinMatrixCount)
		return;

	float4x4 SkinMatrix = LoadMatrix(Params.SkinMatrices, (uint)JointIndex);
	SkinnedPosition += mul(SkinMatrix, float4(Position, 1.0f)) * Weight;
	SkinnedNormal += mul((float3x3)SkinMatrix, Normal) * Weight;
	SkinnedTangent += mul((float3x3)SkinMatrix, Tangent) * Weight;
	AppliedWeight += Weight;
}

void ApplyMorphs(uint VertexIndex, inout float3 Position, inout float3 Normal, inout float3 Tangent)
{
	if (Params.MorphDeltas == 0 || Params.MorphWeights == 0 || Params.MorphWeightCount == 0)
		return;

	for (uint MorphIndex = 0; MorphIndex < Params.MorphWeightCount; MorphIndex++)
	{
		const float Weight = LoadFloat(Params.MorphWeights, MorphIndex);
		if (abs(Weight) <= 0.000001f)
			continue;

		const uint DeltaIndex = MorphIndex * Params.VertexCount + VertexIndex;
		const uint64_t DeltaAddress = Params.MorphDeltas + DeltaIndex * 36;
		Position += vk::RawBufferLoad<float3>(DeltaAddress + 0) * Weight;
		Normal += vk::RawBufferLoad<float3>(DeltaAddress + 12) * Weight;
		Tangent += vk::RawBufferLoad<float3>(DeltaAddress + 24) * Weight;
	}
}

[numthreads(64, 1, 1)]
void main(uint3 ThreadId : SV_DispatchThreadID)
{
	uint VertexIndex = ThreadId.x;
	if (VertexIndex >= Params.VertexCount)
		return;

	float3 PreviousPosition = LoadFloat3(Params.OutVertices, VertexIndex);
	StoreFloat3(Params.OutPrevVertices, VertexIndex, PreviousPosition);

	float3 Position = LoadFloat3(Params.BaseVertices, VertexIndex);
	float3 Normal = LoadFloat3(Params.BaseNormals, VertexIndex);
	float4 TangentAndSign = LoadFloat4(Params.BaseTangents, VertexIndex);
	float3 Tangent = TangentAndSign.xyz;

	ApplyMorphs(VertexIndex, Position, Normal, Tangent);

	const bool HasSkinning = Params.BoneIndices != 0 && Params.BoneWeights != 0 && Params.SkinMatrices != 0 && Params.SkinMatrixCount > 0;
	if (!HasSkinning)
	{
		StoreFloat3(Params.OutVertices, VertexIndex, Position);
		StoreFloat3(Params.OutNormals, VertexIndex, dot(Normal, Normal) > 0.000001f ? normalize(Normal) : Normal);
		StoreFloat4(Params.OutTangents, VertexIndex, float4(dot(Tangent, Tangent) > 0.000001f ? normalize(Tangent) : Tangent, TangentAndSign.w));
		return;
	}

	int4 BoneIndices = LoadInt4(Params.BoneIndices, VertexIndex);
	float4 BoneWeights = LoadFloat4(Params.BoneWeights, VertexIndex);
	float WeightSum = BoneWeights.x + BoneWeights.y + BoneWeights.z + BoneWeights.w;
	if (WeightSum <= 0.000001f)
	{
		StoreFloat3(Params.OutVertices, VertexIndex, Position);
		StoreFloat3(Params.OutNormals, VertexIndex, normalize(Normal));
		StoreFloat4(Params.OutTangents, VertexIndex, float4(normalize(Tangent), TangentAndSign.w));
		return;
	}

	BoneWeights /= WeightSum;

	float4 SkinnedPosition = float4(0, 0, 0, 0);
	float3 SkinnedNormal = float3(0, 0, 0);
	float3 SkinnedTangent = float3(0, 0, 0);
	float AppliedWeight = 0.0f;

	AccumulateSkin(BoneIndices.x, BoneWeights.x, Position, Normal, Tangent, SkinnedPosition, SkinnedNormal, SkinnedTangent, AppliedWeight);
	AccumulateSkin(BoneIndices.y, BoneWeights.y, Position, Normal, Tangent, SkinnedPosition, SkinnedNormal, SkinnedTangent, AppliedWeight);
	AccumulateSkin(BoneIndices.z, BoneWeights.z, Position, Normal, Tangent, SkinnedPosition, SkinnedNormal, SkinnedTangent, AppliedWeight);
	AccumulateSkin(BoneIndices.w, BoneWeights.w, Position, Normal, Tangent, SkinnedPosition, SkinnedNormal, SkinnedTangent, AppliedWeight);

	if (AppliedWeight <= 0.000001f)
	{
		StoreFloat3(Params.OutVertices, VertexIndex, Position);
		StoreFloat3(Params.OutNormals, VertexIndex, normalize(Normal));
		StoreFloat4(Params.OutTangents, VertexIndex, float4(normalize(Tangent), TangentAndSign.w));
		return;
	}

	const float InvAppliedWeight = 1.0f / AppliedWeight;
	SkinnedPosition *= InvAppliedWeight;
	SkinnedNormal *= InvAppliedWeight;
	SkinnedTangent *= InvAppliedWeight;

	float3 OutNormal = dot(SkinnedNormal, SkinnedNormal) > 0.000001f ? normalize(SkinnedNormal) : normalize(Normal);
	float3 OutTangent = dot(SkinnedTangent, SkinnedTangent) > 0.000001f ? normalize(SkinnedTangent) : normalize(Tangent);

	StoreFloat3(Params.OutVertices, VertexIndex, SkinnedPosition.xyz);
	StoreFloat3(Params.OutNormals, VertexIndex, OutNormal);
	StoreFloat4(Params.OutTangents, VertexIndex, float4(OutTangent, TangentAndSign.w));
}
