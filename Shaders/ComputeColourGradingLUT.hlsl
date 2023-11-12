RWTexture3D<float4> LUT;

[numthreads(8,8,8)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float3 neutral = float3(dispatchThreadId) / 32.0f;
	LUT[dispatchThreadId] = float4(neutral, 1);
}