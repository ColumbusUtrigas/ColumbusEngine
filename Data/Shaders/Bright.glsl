#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}

#shader fragment

#uniform Texture2D BaseTexture
#uniform float Treshold

in vec2 UV;

float luma(vec3 color)
{
	return dot(color, vec3(0.299, 0.587, 0.114));
}

void main(void)
{
	vec4 color = Sample2D(BaseTexture, UV);
	FragData[0] = luma(color.rgb) > Treshold ? color : vec4(0, 0, 0, 1);
}


