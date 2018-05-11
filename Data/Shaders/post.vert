in vec3 aPos;
in vec2 aUV;

out vec2 UV;

void main(void)
{
	//Position = ftransform();
	Position = vec4(aPos, 1.0);
	UV = aUV;
	//UV = gl_MultiTexCoord0.xy;
}



