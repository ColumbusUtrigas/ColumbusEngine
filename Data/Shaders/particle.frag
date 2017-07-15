#version 130

varying vec2 varTexCoord;

uniform vec4 uColor;
uniform sampler2D uTex;

void main()
{
	gl_FragColor = uColor * texture(uTex, varTexCoord);
	gl_FragColor = vec4(1, 1, 1, 1);
}







