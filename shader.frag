#version 130

varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;

struct Material
{
	vec4 color;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	sampler2D diffuseTex;
	sampler2D specularTex;
};

struct Light
{
	vec3 color;
	vec3 pos;
};

uniform Material uMaterial;
uniform Light uLight;

vec4 texMap;
vec3 specMap;
vec3 ambient;
vec3 diffuse;
vec3 specular;
vec4 color;

void Init()
{
	texMap = texture(uMaterial.diffuseTex, texCoord);
	specMap = vec3(texture(uMaterial.specularTex, texCoord));
}

vec3 Ambient()
{
	return uMaterial.color.xyz * uMaterial.ambient * uLight.color;
}

void main()
{
	Init();

	vec3 lightDir = normalize(uLight.pos - varFragPos);  
	float diff = max(dot(varNormal, lightDir), 0.0);
	vec3 diffuse = diff * uLight.color;

	vec3 viewDir = normalize(uLight.pos - varFragPos);
	vec3 reflectDir = reflect(-lightDir, varNormal);  

	
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = uMaterial.specular * spec * uLight.color;

	if(specMap.xyz != vec3(0))
		specular *= specMap;

	if(texMap.xyz != vec3(0))
		color = uMaterial.color * texMap * vec4(Ambient() + diffuse + specular, 1.0);
	else
		color = uMaterial.color * vec4(Ambient() + diffuse + specular, 1.0);

	gl_FragColor = color;
}









