#shader vertex

#attribute vec2 aPos 0
#attribute vec2 aUV  1

out vec2 UV;

void main(void)
{
	Position = vec4(aPos, 0, 1);
	UV = aUV;
}

#shader fragment

#uniform Texture2D BaseTexture
#uniform Texture2D Blur
#uniform float Intensity

in vec2 UV;

void main()
{
	vec3 hdrColor = texture(BaseTexture, UV).rgb;      
	vec3 bloomColor = texture(Blur, UV).rgb * Intensity;
	hdrColor += bloomColor;
	FragColor = vec4(hdrColor, 1.0);
}


