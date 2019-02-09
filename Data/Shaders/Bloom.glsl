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
	//@Uniform HorizontalBlur
	//@Uniform VerticalBlur

	uniform sampler2D BaseTexture;
	uniform sampler2D HorizontalBlur;
	uniform sampler2D VerticalBlur;

	in vec2 UV;

	void main()
	{
		FragData[0] = texture(BaseTexture, UV);
	}

#endif




