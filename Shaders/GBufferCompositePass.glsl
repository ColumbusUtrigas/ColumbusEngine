#version 460 core

#ifdef VERTEX_SHADER
	layout (location = 0) out vec2 texcoords;

	const vec3 verts[3] = vec3[](
		vec3(-3, -1, 0),
		vec3(1, 3, 0),
		vec3(1, -1, 0)
	);

	void main()
	{
		gl_Position = vec4(verts[gl_VertexIndex], 1);
		texcoords = 0.5 * gl_Position.xy + vec2(0.5);
	}
#endif

#ifdef PIXEL_SHADER
	layout(location = 0) in vec2 texcoords;

	layout(binding = 0, set = 0) uniform sampler2D InputAlbedo;
	layout(binding = 1, set = 0) uniform sampler2D InputNormal;
	layout(binding = 2, set = 0, rgba16f) uniform image2D InputShadowBuffer;

	layout(location = 0) out vec4 RT0;

	void main()
	{
		// TODO:
		vec3 normal = texture(InputNormal, texcoords).rgb;
		float ndotl = max(dot(normal, normalize(vec3(1, 1, 0)) ), 0);
		float shadow = imageLoad(InputShadowBuffer, ivec2(texcoords * vec2(1280, 720))).r;

		RT0 = texture(InputAlbedo, texcoords) * ndotl * shadow;
	}
#endif
