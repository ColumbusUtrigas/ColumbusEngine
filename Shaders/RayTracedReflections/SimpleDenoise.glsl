#version 460 core
#extension GL_GOOGLE_include_directive : require

layout(binding = 0, set = 0, rgba16f) uniform image2D InputReflection;
layout(binding = 1, set = 0, rgba16f) uniform image2D OutputReflection;

layout(push_constant) uniform Params
{
	ivec2 ImageSize;
} Parameters;

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
void main()
{
	const int KernelSize = 5;
	const int KernelAxis = 2;
	const float Kernel[] = { 0.3829, 0.2417, 0.0606, 0.0060, 0.0002 };

	ivec2 Pixel = ivec2(gl_GlobalInvocationID.xy);
	ivec2 Size = Parameters.ImageSize;

	vec3 FinalResult = vec3(0,0,0);

	// simple box blur
	#if 0
	for (int i = -KernelAxis; i <= KernelAxis; i++)
	{
		for (int j = -KernelAxis; j <= KernelAxis; j++)
		{
			ivec2 SampleLocation = Pixel + ivec2(i, j);
			SampleLocation = clamp(SampleLocation, ivec2(0,0), Size);

			FinalResult += imageLoad(InputReflection, SampleLocation).rgb;
		}
	}
	FinalResult /= float(KernelSize * KernelSize);
	#endif

	// simple gaussian blur
	#if 1
	for (int i = -KernelAxis; i <= KernelAxis; i++)
	{
		for (int j = -KernelAxis; j <= KernelAxis; j++)
		{
			ivec2 SampleLocation = Pixel + ivec2(i, j);
			SampleLocation = clamp(SampleLocation, ivec2(0,0), Size);

			FinalResult += imageLoad(InputReflection, SampleLocation).rgb * Kernel[i+KernelAxis] * Kernel[j+KernelAxis];
		}
	}
	#endif

	imageStore(OutputReflection, Pixel, vec4(FinalResult, 1));
}
