#version 460 core
#extension GL_GOOGLE_include_directive : require

#ifdef VERTEX_SHADER
	const vec2 pos[3] = vec2[3](
		vec2(-1, -1),
		vec2(-1, 3),
		vec2(3, -1)
	);

	void main()
	{
		gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
	}
#endif

#ifdef PIXEL_SHADER
	#define RADIANCE_CACHE_BINDING 0
	#define RADIANCE_CACHE_SET 0
	#include "RadianceCache.glsl"

	layout(binding = 1, set = 0) uniform sampler2D GBufferWorldPosition;

	layout(location = 0) out vec4 RT0;

	layout(push_constant) uniform params
	{
		vec3 CameraPosition;
	} Params;

	vec3 TonemapReinhard(vec3 x)
	{
		return x / (x + 1);
	}

	void main()
	{
		const vec3 Position = texelFetch(GBufferWorldPosition, ivec2(gl_FragCoord.xy), 0).xyz;

		RT0 = vec4(TonemapReinhard(SampleRadianceCache(Params.CameraPosition, Position)), 1);
		//RT0 = vec4(SampleRadianceCache(Params.CameraPosition, Position), 1);
	}
#endif