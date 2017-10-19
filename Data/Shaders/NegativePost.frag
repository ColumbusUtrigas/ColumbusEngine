#version 130

uniform sampler2D uFrame;

out vec3 FinalColor;

void main(void)
{
	vec2 UV = gl_TexCoord[0].xy;
	vec3 frame = texture(uFrame, UV).xyz;
	FinalColor = vec3(1) - frame;
}






