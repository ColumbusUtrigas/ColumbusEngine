#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}

#shader fragment

#uniform Texture2D BaseTexture
#uniform bool Horizontal

in vec2 UV;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
	const float LOD = 0.0;

	vec2 tex_offset = 1.0 / textureSize(BaseTexture, 0);
	vec3 result = Sample2D(BaseTexture, UV).rgb * weight[0];

	if (Horizontal)
	{
		for (int i = 1; i < 5; ++i)
		{
			result += Sample2D(BaseTexture, UV + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			result += Sample2D(BaseTexture, UV - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else
	{
		for (int i = 1; i < 5; ++i)
		{
			result += Sample2D(BaseTexture, UV + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			result += Sample2D(BaseTexture, UV - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
		}
	}

	FragData[0] = vec4(result, 1.0);
}


