#define LIGHT_NUM 4

in vec3 varPos;
in vec2 varUV;
in vec3 varNormal;
in vec3 varFragPos;
in mat3 varTBN;

struct Material
{
	sampler2D diffuseMap;
	sampler2D specularMap;
	sampler2D normalMap;
};

struct Camera
{
	vec3 pos;
};

uniform samplerCube uReflectionMap;
uniform Material uMaterial;
uniform Camera uCamera;
uniform float MaterialUnif[15];
uniform float LightUnif[120];

vec4 DiffuseMap;
vec3 SpecularMap;
vec3 NormalMap;

bool IsSpecularMap = false;

vec3 Normal;

vec3 AmbientColor = vec3(0);
vec3 DiffuseColor = vec3(0);
vec3 SpecularColor = vec3(0);
vec3 CubemapColor = vec3(0);
vec4 Lighting = vec4(1);

mat3 TBN;

void Init(void);
void Light(int id);
void Cubemap(void);
void Final(void);

void main(void)
{
	Init();


	if (MaterialUnif[14] != 0.0)
	{
		Light(0);
		Light(1);
		Light(2);
		Light(3);
		Light(4);
		Light(5);
		Light(6);
		Light(7);

		Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);
	}
	
	Cubemap();

	Final();
}

void Init(void)
{
	DiffuseMap = texture(uMaterial.diffuseMap, varUV);
	SpecularMap = vec3(texture(uMaterial.specularMap, varUV));
	NormalMap = vec3(texture(uMaterial.normalMap, varUV));

	if (DiffuseMap.w <= 0.1) discard;

	TBN = varTBN;

	if (textureSize(uMaterial.specularMap, 1).xy != vec2(0))
		IsSpecularMap = true;

	if (NormalMap != vec3(0))
		Normal = normalize(NormalMap * 2.0 - 1.0) * TBN;
	else
		Normal = varNormal;
}

void Light(int id)
{
	vec4 MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
	vec3 MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
	vec3 MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);
	vec3 MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);

	int offset = id * 15;

	vec3 LightColor = vec3(LightUnif[0 + offset], LightUnif[1 + offset], LightUnif[2 + offset]);
	vec3 LightPos = vec3(LightUnif[3 + offset], LightUnif[4 + offset], LightUnif[5 + offset]);
	vec3 LightDir = vec3(LightUnif[6 + offset], LightUnif[7 + offset], LightUnif[8 + offset]);
	float LightType = LightUnif[9 + offset];
	float LightConstant = LightUnif[10 + offset];
	float LightLinear = LightUnif[11 + offset];
	float LightQuadratic = LightUnif[12 + offset];
	float LightInnerAngle = LightUnif[13 + offset];
	float LightOuterAngle = LightUnif[14 + offset];

	if (LightType == -1) return;

	vec3 lightDir;

	float attenuation = 0.0;

	switch (int(LightType))
	{
	case 0:
		lightDir = normalize(-LightPos);
		break;
	default:
		lightDir = normalize(-LightPos + varPos);
		break;
	};

	vec3 viewDir = normalize(uCamera.pos - varFragPos);

	float diff = max(0.0, dot(Normal, -lightDir));

	vec3 reflect = normalize(reflect(lightDir, Normal));
	float spec = pow(max(0.0, dot(viewDir, reflect)), 32);
	vec3 specular = MaterialSpecular * LightColor * spec * 0.5;

	vec3 tmpAmbient = vec3(0);
	vec3 tmpDiffuse = vec3(0);
	vec3 tmpSpecular = vec3(0);

	tmpAmbient = MaterialAmbient * LightColor * vec3(MaterialColor);
	tmpDiffuse = LightColor * MaterialDiffuse * diff * MaterialColor.xyz;
	
	if (IsSpecularMap)
		tmpSpecular = specular * MaterialSpecular * MaterialColor.xyz * SpecularMap;
	else
		tmpSpecular = specular * MaterialSpecular * MaterialColor.xyz;

	if (int(LightType) > 0)
	{
		float distance = length(LightPos - varPos);
		attenuation = 1.0 / (LightConstant +
							LightLinear * distance +
							LightQuadratic * (distance * distance));

		tmpAmbient *= attenuation;
		tmpDiffuse *= attenuation;
		tmpSpecular *= attenuation;
	}

	AmbientColor += tmpAmbient;
	DiffuseColor += tmpDiffuse;
	SpecularColor += tmpSpecular;
}

void Cubemap(void)
{
	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = normalize(reflect(I, Normal));
   	CubemapColor = textureCube(uReflectionMap, -vec3(R.x, R.y, R.z)).rgb * MaterialUnif[13];

   	if (IsSpecularMap)
		CubemapColor *= SpecularMap;

	vec4 MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);

	CubemapColor *= MaterialColor.xyz;
}

void Final(void)
{
	if (DiffuseMap.xyz != vec3(0))
	{
		if (CubemapColor != vec3(0))
			FragColor = Lighting * DiffuseMap + vec4(CubemapColor, 1.0);
		else
			FragColor = Lighting * DiffuseMap;
	}
	else
	{
		if (CubemapColor != vec3(0))
			FragColor = Lighting + vec4(CubemapColor, 1.0);
		else
			FragColor = Lighting;
	}
}





