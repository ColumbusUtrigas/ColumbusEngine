#version 130

varying vec2 texCoord;
varying vec3 varNormal;

struct Material
{
	vec4 color;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	sampler2D diffuseTex;
};

uniform Material uMaterial;

vec4 texMap;

void main()
{
	texMap = texture(uMaterial.diffuseTex, texCoord);
	if(texMap != vec4(0, 0, 0, 0))
		gl_FragColor = uMaterial.color * texMap;
	else
		gl_FragColor = uMaterial.color;
}









