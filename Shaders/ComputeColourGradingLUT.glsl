#version 460 core

layout(binding = 0, set = 0, rgba16f) writeonly uniform image3D LUT;

void main()
{
	vec3 neutral = gl_GlobalInvocationID.xyz / 32.0f;
	imageStore(LUT, ivec3(gl_GlobalInvocationID.xyz), vec4(neutral, 1));
}
