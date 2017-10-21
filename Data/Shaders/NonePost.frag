#version 130

uniform sampler2D uColor;
uniform sampler2D uDepth;

out vec4 FinalColor;

vec2 UV = gl_TexCoord[0].xy;

void main()
{
	float d = pow(texture(uDepth, UV).x, 256);

	FinalColor = vec4(d, d, d, 1.0);
	FinalColor = vec4(texture(uColor, UV).rgb, 1.0);
}
