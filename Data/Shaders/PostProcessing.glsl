#ifdef VertexShader

	out vec2 UV;

	void main(void)
	{
		Position = ftransform();
		UV = gl_MultiTexCoord0.xy;
	}

#endif

#ifdef FragmentShader

	//@Uniform BaseTexture

	uniform sampler2D BaseTexture;

	in vec2 UV;

	void main()
	{
		FragData[0] = vec4(texture(BaseTexture, UV).rgb, 1);
	}

#endif








