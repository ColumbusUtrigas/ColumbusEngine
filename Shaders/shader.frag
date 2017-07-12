#version 130

varying vec3 varPos;
varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;

uniform samplerCube uReflectionMap;


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
	vec3 direction;
	vec3 color;

	int type; //0 = DIRECTIONAL; 1 = POINT; 2 = SPOT

	float constant;
    float linear;
    float quadratic;
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
vec3 Reflection;

float Attenuation;

void Init()
{
	DiffuseMap = texture(uMaterial.diffuseMap, texCoord);
	SpecularMap = vec3(texture(uMaterial.specularMap, texCoord));


	if(uLight.type == 1)
	{
		float distance = length(uLight.pos - varFragPos);
		Attenuation = 1.0 / (uLight.constant + uLight.linear * distance + uLight.quadratic * (distance * distance)); 
	}
}

vec3 GetAmbient()
{
	return uMaterial.color.xyz * uMaterial.ambient * uLight.color;
}

vec3 GetDiffuse()
{
	if(uLight.type == 0)
	{
		vec3 lightDir = normalize(-uLight.direction);
		float diff = max(dot(varNormal, lightDir), 0.0);
		vec3 diffuse = diff * uLight.color;
		return diffuse;
	}

	if(uLight.type == 1)
	{
		vec3 lightDir = normalize(uLight.pos - varFragPos);
		float diff = max(dot(varNormal, lightDir), 0.0);
		vec3 diffuse = diff * uLight.color;
		return diffuse;
	}
}

vec3 GetSpecular()
{
	if(uLight.type == 0)
	{
		vec3 lightDir = normalize(-uLight.direction);
		vec3 viewDir = normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, varNormal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * uLight.color;
		return specular;
	}

	if(uLight.type == 1)
	{
		vec3 lightDir = normalize(uLight.pos - varFragPos);
		vec3 viewDir = normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, varNormal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * uLight.color;
		return specular;
	}
}

vec3 GetReflection()
{
	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, varNormal);
   	
   	return textureCube(uReflectionMap, vec3(R.x, R.y, -R.z)).rgb;
}

void main()
{
	Init();

	Ambient = GetAmbient();
	Diffuse = GetDiffuse();
	Specular = GetSpecular();
	Reflection = GetReflection();

	Ambient *= Attenuation;
	Diffuse *= Attenuation;
	Specular *= Attenuation;

	if (SpecularMap != vec3(0))
	{
		Specular *= SpecularMap;
		Reflection *= SpecularMap;
	}

	Color = vec4(Ambient + Diffuse + Specular, 1.0);

	if (DiffuseMap.xyz != vec3(0))
		Color *= DiffuseMap;

	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, varNormal);
   	
    gl_FragColor = Color + (vec4(Reflection, 1.0) * 0.9);
    
    //gl_FragColor = Color;
    
	//gl_FragColor = vec4(textureCube(uReflectionMap, vec3(R.x, R.y, -R.z)).rgb, 1.0) * 0.4;
}









