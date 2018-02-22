#version 130

varying vec3 varPos;
varying vec2 texCoord;
varying vec3 varNormal;
varying vec3 varFragPos;
varying mat3 varTBN;

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

//0..2 - Light Color
//3..5 - Light Pos
//6..8 - Light Dir
//9 - Light Type
//10 - Light Constant
//11 - Light Linear
//12 - Light Quadratic
//13 - Light Inner Angle
//14 - Light Outer Angle
uniform float LightUnif[15];

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

vec3 LightColor;
vec3 LightPos;
vec3 LightDirection;
float LightType;
float LightConstant;
float LightLinear;
float LightQuadratic;
float LightCutoff;
float LightOuterCutoff;

float Attenuation;

void Init()
{
	DiffuseMap = texture(uMaterial.diffuseMap, texCoord);
	SpecularMap = vec3(texture(uMaterial.specularMap, texCoord));
	NormalMap = vec3(texture(uMaterial.normalMap, texCoord));
	
	if (NormalMap != vec3(0))
	{
		Normal = NormalMap;
		Normal = normalize(Normal * 2.0 - 1.0);
		//Normal = Normal * varTBN;
	}
	else
		Normal = varNormal;
		
	MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
	MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
	MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);
	MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);
	MaterialReflection = MaterialUnif[13];
	
	LightColor = vec3(LightUnif[0], LightUnif[1], LightUnif[2]);
	LightPos = vec3(LightUnif[3], LightUnif[4], LightUnif[5]);
	LightDirection = vec3(LightUnif[6], LightUnif[7], LightUnif[8]);
	LightType = LightUnif[9];
	LightConstant = LightUnif[10];
	LightLinear = LightUnif[11];
	LightQuadratic = LightUnif[12];
	LightCutoff = LightUnif[13];
	LightOuterCutoff = LightUnif[14];


	if(LightType == 1 || LightType == 2)
	{
		float distance = length(LightPos - varFragPos);
		Attenuation = 1.0 / (LightConstant + LightLinear * distance + LightQuadratic * (distance * distance));
	}
}

vec3 GetAmbient()
{
	return MaterialColor.xyz * MaterialAmbient * LightColor;
}

vec3 GetDiffuse()
{
	if(LightType == 0)
	{
		vec3 lightDir = varTBN * normalize(-LightDirection);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = diff * LightColor;
		return diffuse * MaterialDiffuse;
	}

	if(LightType == 1)
	{
		vec3 lightDir = varTBN * normalize(LightPos - varFragPos);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = diff * LightColor;
		return diffuse * MaterialDiffuse;
	}
	
	if (LightType == 2)
	{
		vec3 lightDir = varTBN * normalize(LightPos - varFragPos);
		float diff = max(dot(Normal, lightDir), 0.0);
		vec3 diffuse = diff * LightColor;
		return diffuse * MaterialDiffuse;
	}
}

vec3 GetSpecular()
{
	if(LightType == 0)
	{
		vec3 lightDir = varTBN * normalize(-LightDirection);
		vec3 viewDir = varTBN * normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * LightColor;
		return specular * MaterialSpecular;
	}

	if(LightType == 1)
	{
		vec3 lightDir = varTBN * normalize(LightPos - varFragPos);
		vec3 viewDir = varTBN * normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * LightColor;
		return specular * MaterialSpecular;
	}
	
	if(LightType == 2)
	{
		vec3 lightDir = varTBN * normalize(LightPos - varFragPos);
		vec3 viewDir = varTBN * normalize(uCamera.pos - varFragPos);
		vec3 reflectDir = reflect(-lightDir, Normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
		vec3 specular = 0.5 * spec * LightColor;
		return specular * MaterialSpecular;
	}
}

vec3 GetReflection()
{
	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, Normal);

   	vec3 a = textureCube(uReflectionMap, vec3(R.x, R.y, R.z)).rgb;
   	/*vec3 b = textureCube(uReflectionMap, vec3(R.x + 0.01, R.y, R.z)).rgb;
   	vec3 c = textureCube(uReflectionMap, vec3(R.x - 0.01, R.y, R.z)).rgb;
   	vec3 d = textureCube(uReflectionMap, vec3(R.x, R.y + 0.01, R.z)).rgb;
   	vec3 e = textureCube(uReflectionMap, vec3(R.x, R.y - 0.01, R.z)).rgb;*/
   	
   	//a = mix(mix(b, c, 0.5), mix(d, e, 0.5), 0);
   	
   	//a = mix(mix(mix(b, c, 0.5), mix(d, e, 0.5), 0.5), a, 0);
   	
   	return vec3(0);
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
	Reflection *= Attenuation;

	if (SpecularMap != vec3(0))
	{
		Specular *= SpecularMap;
		Reflection *= SpecularMap;
	}

	Color = vec4(Ambient + Diffuse + Specular, 1.0) * MaterialColor;

	/*if (DiffuseMap.xyz != vec3(0))
		Color *= DiffuseMap;*/

	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = reflect(I, varNormal);

    //gl_FragColor = Color + (vec4(Reflection, 1.0) * 0.9);
    //gl_FragColor = mix(Color, vec4(Reflection, 1.0), 0.2);
    //gl_FragColor = Color + vec4(Reflection * 1, 1.0);
    
    gl_FragColor = Color * DiffuseMap + vec4(Reflection * 0.2, 1.0);

    //gl_FragColor = vec4(Reflection, 1.0);
    
    //gl_FragColor = vec4(varTBN[0], 1.0);

	//gl_FragColor = vec4(textureCube(uReflectionMap, vec3(R.x, R.y, -R.z)).rgb, 1.0) * 0.4;
}
