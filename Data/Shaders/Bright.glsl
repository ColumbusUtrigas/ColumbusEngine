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

	#define TRESHOLD 0.5

	in vec2 UV;

	float luma(vec3 color) {
  return dot(color, vec3(0.299, 0.587, 0.114));
}


	void main(void)
	{
		vec4 color = texture(BaseTexture, UV);
		FragData[0] = luma(color.rgb) > TRESHOLD ? color : vec4(0, 0, 0, 1);
	}

#endif



