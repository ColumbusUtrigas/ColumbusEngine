#ifdef VertexShader

	in vec3 aPos;
	in vec2 aUV;

	out vec2 UV;

	void main(void)
	{
		Position = vec4(aPos, 1.0);
		UV = aUV;
	}

#endif

#ifdef FragmentShader

	uniform sampler2D uColor;
	uniform sampler2D uDepth;
	uniform vec2 uResolution;

	in vec2 UV;

	float blurSizeH = 1.0 / uResolution.x;
	float blurSizeV = 1.0 / uResolution.y;

	vec3 GaussianBlur(vec2 size)
	{
		vec4 sum = vec4(0.0);

		float sX = blurSizeH * size.x;
		float sY = blurSizeV * size.y;

		for (int x = -4; x <= 4; x++)
		{
			for (int y = -4; y <= 4; y++)
			{
				sum += texture(uColor, vec2(UV.x + x * sX, UV.y + y * sY)) / 81.0;
			}
		}

		return sum.xyz;
	}

	void main()
	{
		//float d = pow(texture(uDepth, UV).x, 256);

		FragColor = vec4(texture(uColor, UV).rgb, 1.0);
	}

#endif








