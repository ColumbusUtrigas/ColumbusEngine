struct Material
{
	sampler2D DiffuseMap;
	sampler2D SpecularMap;
	sampler2D NormalMap;
	samplerCube ReflectionMap;

	vec4 Color;
	vec3 AmbientColor;
	vec3 DiffuseColor;
	vec3 SpecularColor;
	float ReflectionPower;
	bool Lighting;
};

uniform Material uMaterial;

varying vec3 varColor;
varying vec2 varUV;

void main()
{
	FragColor = texture(uMaterial.DiffuseMap, varUV);
}


