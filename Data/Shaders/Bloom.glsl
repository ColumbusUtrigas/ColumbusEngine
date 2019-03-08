#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}

#shader fragment

#uniform Texture2D BaseTexture
#uniform Texture2D HorizontalBlur
#uniform Texture2D VerticalBlur

in vec2 UV;

void main()
{
	FragData[0] = texture(BaseTexture, UV);
}


