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
	uniform sampler2D uNormal;
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
		float factor = 1.0 / 81.0;

		for (int x = -4; x <= 4; x++)
		{
			for (int y = -4; y <= 4; y++)
			{
				sum += texture(uColor, vec2(UV.x + x * sX, UV.y + y * sY)) * factor;
			}
		}

		return sum.xyz;
	}

	vec3 DecodeNormal(in vec2 v)
	{
		vec2 fenc = 4.0 * v - 2.0;
		float f = dot(fenc, fenc);
		return vec3(fenc * sqrt(1.0 - 0.25 * f), 1.0 - 0.5 * f);
	}

	void main()
	{
		//float d = 2 * texture(uDepth, UV).x - 1;
		//FragColor = vec4(vec3(pow(d, 64)), 1);
		//FragColor = vec4(GaussianBlur(vec2(d)), 1);
		FragColor = vec4(texture(uColor, UV).rgb, 1);
		//FragColor = vec4(DecodeNormal(texture(uNormal, UV).rg), 1);
	}

#endif








