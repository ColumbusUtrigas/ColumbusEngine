#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}

#shader fragment

#uniform Texture2D BaseTexture

in vec2 UV;

void main()
{
	FragData[0] = vec4(Sample2D(BaseTexture, UV).rgb, 1);
}


