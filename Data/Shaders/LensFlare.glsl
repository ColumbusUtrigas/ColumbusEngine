#ifdef VertexShader

	out vec2 UV;

	void main(void)
	{
		Position = ftransform();
		UV = gl_MultiTexCoord0.xy;
	}

#endif

#ifdef FragmentShader

	//@Uniform Texture
	uniform sampler2D Texture;

	in vec2 UV;

	void main()
	{
		FragData[0] = texture(Texture, UV);
	}

#endif



