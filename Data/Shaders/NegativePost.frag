uniform sampler2D uFrame;

void main(void)
{
	vec2 UV = gl_TexCoord[0].xy;
	vec3 frame = texture(uFrame, UV).xyz;
	FragColor = vec4(vec3(1) - frame, 1.0);
}






