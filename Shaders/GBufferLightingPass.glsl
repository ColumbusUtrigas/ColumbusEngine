#version 460 core

#ifdef COMPUTE_SHADER
	layout(binding = 0, set = 0) uniform sampler2D GBufferAlbedo;
	layout(binding = 1, set = 0) uniform sampler2D GBufferNormal;
	layout(binding = 2, set = 0, rgba16f) uniform image2D LightingOutput;
	layout(binding = 3, set = 0, r8) uniform image2D ShadowBuffer;
	layout(binding = 4, set = 0) uniform sampler2D GBufferWorldPosition;

	// TODO: use GPUScene common definitions
	struct GPULight
	{
		vec4 Position;
		vec4 Direction;
		vec4 Color;
		uint Type;
		float Range;
	};

	#define GPULIGHT_DIRECTIONAL 0
	#define GPULIGHT_POINT 1
	#define GPULIGHT_SPOT 2
	#define GPULIGHT_RECTANGLE 3
	#define GPULIGHT_SPHERE 4

	layout(binding = 5, set = 0) readonly buffer LightsBuffer {
		GPULight Lights[];
	} GPUSceneLights;

	// TODO: create a global GPUScene/View cbuffer
	layout(push_constant) uniform Params
	{
		uint LightsCount;
	} Parameters;

	layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

	void main()
	{
		vec2 UV = (vec2(gl_GlobalInvocationID.xy) + 0.5) / vec2(gl_NumWorkGroups.xy) / vec2(gl_WorkGroupSize.xy);
		if (UV.x >= 1 && UV.y >= 1)
			return;

		vec3 Albedo = texture(GBufferAlbedo, UV).rgb;
		vec3 N = texture(GBufferNormal, UV).rgb;
		vec3 WorldPosition = texture(GBufferWorldPosition, UV).rgb;
		vec3 LightingSum = vec3(0);

		// TODO: move lighting functions to a common header
		for (uint i = 0; i < Parameters.LightsCount; i++)
		{
			GPULight Light = GPUSceneLights.Lights[i];

			float Shadow = imageLoad(ShadowBuffer, ivec2(gl_GlobalInvocationID.xy)).r;
			vec3 LightDir = Light.Direction.xyz;

			if (Light.Type != GPULIGHT_DIRECTIONAL)
				Shadow = 1; // TODO: support per-light shadows, currently only from directional light

			float Attenuation = 0;
			float Distance = distance(WorldPosition, Light.Position.xyz);
			float LightRange = Light.Range;

			switch (Light.Type)
			{
			case GPULIGHT_DIRECTIONAL:
				Attenuation = 1; // directional light doesn't attenuate
				break;
			case GPULIGHT_POINT:
				Attenuation = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0);
				Attenuation *= Attenuation;
				LightDir = -normalize(WorldPosition - Light.Position.xyz);
				break;
			default: // TODO: support other light types
				break;
			}

			LightingSum += vec3(max(dot(N, LightDir), 0)) * Albedo * Shadow * Attenuation * Light.Color.rgb;
		}

		imageStore(LightingOutput, ivec2(gl_GlobalInvocationID.xy), vec4(LightingSum, 1));
	}
#endif
