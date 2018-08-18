#ifdef VertexShader

	varying vec2 varUV;

	void main(void)
	{
		Position = ftransform();
		varUV = gl_MultiTexCoord0.xy;
	}

#endif

#ifdef FragmentShader

	varying vec2 varUV;

	uniform vec4 uColor;
	uniform sampler2D uTexture;

	const float Contrast = 10;
	const float Glow = 2;

	//@Uniform uColor
	//@Uniform uTexture
	void main(void)
	{
		vec4 C = texture(uTexture, varUV);
		vec4 C1 = clamp((C - 0.5) * Contrast, 0, 1);
		vec4 C2 = clamp(1 - (C - 0.5) / Glow, 0, 1);

		FragColor = C1;
	}

#endif














