#version 130

varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;

uniform sampler2D diffTex;
uniform sampler2D specTex;

struct Material
{
	sampler2D diffuseMap;
	sampler2D specularMap;

	vec4 color;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Light
{
	vec3 pos;
	vec3 color;
};

struct Camera
{
	vec3 pos;
};

uniform Material uMaterial;
uniform Light uLight;
uniform Camera uCamera;

vec4 DiffuseMap;
vec3 SpecularMap;

vec4 Color;
vec3 Ambient;
vec3 Diffuse;
vec3 Specular;

void Init()
{
	DiffuseMap = texture(uMaterial.diffuseMap, texCoord);
	SpecularMap = vec3(texture(uMaterial.specularMap, texCoord));
}

vec3 GetAmbient()
{
	return uMaterial.color.xyz * uMaterial.ambient * uLight.color;
}

vec3 GetDiffuse()
{
	vec3 lightDir = normalize(uLight.pos - varFragPos);
	float diff = max(dot(varNormal, lightDir), 0.0);
	vec3 diffuse = diff * uLight.color;
	return diffuse;
}

vec3 GetSpecular()
{
	vec3 lightDir = normalize(uLight.pos - varFragPos);
	vec3 viewDir = normalize(uCamera.pos - varFragPos);
	vec3 reflectDir = reflect(-lightDir, varNormal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = 0.5 * spec * uLight.color;
	return specular;
}

void main()
{
	Init();

	Ambient = GetAmbient();
	Diffuse = GetDiffuse();
	Specular = GetSpecular();

	if (SpecularMap != vec3(0))
		Specular *= SpecularMap;

	Color = vec4(Ambient + Diffuse + Specular, 1.0);

	if (DiffuseMap.xyz != vec3(0))
		Color *= DiffuseMap;

	gl_FragColor = Color;
}









