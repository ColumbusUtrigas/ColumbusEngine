RWTexture3D<float> LUT;

void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float3 neutral = float3(dispatchThreadId) / 32.0f;
	LUT[dispatchThreadId] = neutral;
}