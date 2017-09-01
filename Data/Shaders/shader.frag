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
	sampler2D normalMap;

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


//0..3 - Material Color
//4..6 - Material Ambient
//7..9 - Material Diffuse
//10..12 - Material Specular
uniform float MaterialUnif[14];

vec4 DiffuseMap;
vec3 SpecularMap;
vec3 NormalMap;

vec3 Normal;

vec4 Color;
vec3 Ambient;
vec3 Diffuse;
vec3 Specular;
vec3 Reflection;

vec4 MaterialColor;
vec3 MaterialAmbient;
vec3 MaterialDiffuse;
vec3 MaterialSpecular;
float MaterialReflection;

float Attenuation;

void Init()
{
	DiffuseMap = texture(uMaterial.diffuseMap, texCoord);
	SpecularMap = vec3(texture(uMaterial.specularMap, texCoord));
	NormalMap = vec3(texture(uMaterial.normalMap, texCoord));
	
	if (NormalMap == vec3(0))
		Normal = normalize(NormalMap * 2.0 - 1.0);  
	else
		Normal = varNormal;
		
	MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
	MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
	MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);
	MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);
	MaterialReflection = MaterialUnif[13];


	if(uLight.type == 1)
	{
		float distance = length(uLight.pos - varFragPos);
		Attenuation = 1.0 / (uLight.constant + uLight.linear * distance + uLight.quadratic * (distance * distance));
	}
}

vec3 GetAmbient()
{
	return MaterialColor.xyz * MaterialAmbient * uLight.color;
}

vec3 GetDiffuse()
{
	if(uLight.type == 0)
	{
		vec3 lightDir = normalize(-uLight.direction);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = diff * uLight.color;
		return diffuse * MaterialDiffuse;
	}

	if(uLight.type == 1)
	{
		vec3 lightDir = normalize(uLight.pos - varFragPos);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = diff * uLight.color;
		return diffuse * MaterialDiffuse;
	}
}

vec3 GetSpecular()
{
	if(uLight.type == 0)
	{
		vec3 lightDir = normalize(-uLight.direction);
		vec3 viewDir = normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * uLight.color;
		return specular * MaterialSpecular;
	}

	if(uLight.type == 1)
	{
		vec3 lightDir = normalize(uLight.pos - varFragPos);
		vec3 viewDir = normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * uLight.color;
		return specular * MaterialSpecular;
	}
}

vec3 GetReflection()
{
	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, Normal);

   	vec3 a = textureCube(uReflectionMap, vec3(R.x, R.y, R.z)).rgb;
   	vec3 b = textureCube(uReflectionMap, vec3(R.x + 0.01, R.y, R.z)).rgb;
   	vec3 c = textureCube(uReflectionMap, vec3(R.x - 0.01, R.y, R.z)).rgb;
   	vec3 d = textureCube(uReflectionMap, vec3(R.x, R.y + 0.01, R.z)).rgb;
   	vec3 e = textureCube(uReflectionMap, vec3(R.x, R.y - 0.01, R.z)).rgb;
   	
   	//a = mix(mix(b, c, 0.5), mix(d, e, 0.5), 0);
   	
   	a = mix(mix(mix(b, c, 0.5), mix(d, e, 0.5), 0.5), a, 0);
   	
   	return a;
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

	Color = vec4(Ambient + Diffuse + Specular, 1.0) * MaterialColor;

	if (DiffuseMap.xyz != vec3(0))
		Color *= DiffuseMap;

	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, varNormal);

    //gl_FragColor = Color + (vec4(Reflection, 1.0) * 0.9);
    //gl_FragColor = mix(Color, vec4(Reflection, 1.0), 0.2);
    gl_FragColor = Color + vec4(Reflection * MaterialReflection, 1.0);
    
    //gl_FragColor = vec4(Normal, 1);

    //gl_FragColor = vec4(Reflection, 1.0);

	//gl_FragColor = vec4(textureCube(uReflectionMap, vec3(R.x, R.y, -R.z)).rgb, 1.0) * 0.4;
}
