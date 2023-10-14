#version 460 core

#ifdef COMPUTE_SHADER
	layout(binding = 0, set = 0) uniform sampler2D GBufferAlbedo;
	layout(binding = 1, set = 0) uniform sampler2D GBufferNormal;
	layout(binding = 2, set = 0, rgba16f) uniform image2D LightingOutput;
	layout(binding = 3, set = 0, r8) uniform image2D ShadowBuffer;

	void main()
	{
		vec2 UV = vec2(gl_GlobalInvocationID.xy) / vec2(gl_NumWorkGroups.xy);
		float Shadow = imageLoad(ShadowBuffer, ivec2(gl_GlobalInvocationID.xy)).r;
		vec3 LightDir = normalize(vec3(1,1,1));

		vec3 Albedo = texture(GBufferAlbedo, UV).rgb;
		vec3 N = texture(GBufferNormal, UV).rgb;
		vec3 Lighting = vec3(max(dot(N, LightDir), 0)) * Albedo * Shadow;

		imageStore(LightingOutput, ivec2(gl_GlobalInvocationID.xy), vec4(Lighting, 1));
	}
#endif
