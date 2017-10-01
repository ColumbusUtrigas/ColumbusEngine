#version 130

varying vec3 varPos;
varying vec2 varUV;
varying vec3 varNormal;
varying vec3 varTangent;
varying vec3 varBitangent;
varying vec3 varFragPos;

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

uniform Material uMaterial;
uniform Camera uCamera;
uniform float MaterialUnif[14];
uniform samplerCube uReflectionMap;

vec4 DiffuseMap;
vec3 SpecularMap;
vec3 NormalMap;

vec3 Normal;

vec3 AmbientColor;
vec3 DiffuseColor;
vec3 SpecularColor;
vec3 CubemapColor;

vec4 FinalColor;

mat3 TBN;

void Init(void);
void Ambient(void);
void Diffuse(void);
void Specular(void);
void Cubemap(void);
void Final(void);

void main(void)
{
	Init();

	Ambient();
	Diffuse();
	Specular();
	Cubemap();

	Final();

	gl_FragColor = FinalColor;
}

void Init(void)
{
	DiffuseMap = texture(uMaterial.diffuseMap, varUV);
	SpecularMap = vec3(texture(uMaterial.specularMap, varUV));
	NormalMap = vec3(texture(uMaterial.normalMap, varUV));

	TBN = transpose(mat3(varTangent, varBitangent, varNormal));

	if (NormalMap != vec3(0))
		Normal = normalize(NormalMap * 2.0 - 1.0) * TBN;
	else
		Normal = varNormal;
}

void Ambient(void)
{
	vec4 MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
	vec3 MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
	AmbientColor = MaterialAmbient * vec3(1, 1, 1) * vec3(MaterialColor);
}

void Diffuse(void)
{
	vec3 MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);

<<<<<<< HEAD
	vec3 lightDir = normalize(vec3(-uCamera.pos));
	vec3 viewDir = normalize(uCamera.pos - varPos);
	float diff = max(0.0, dot(Normal, -lightDir));
	DiffuseColor = vec3(1) * MaterialDiffuse * diff;
=======
	vec3 lightDir = normalize(vec3(1, 1, 1));
	float diff = max(dot(Normal, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1, 1, 1);
	DiffuseColor = diffuse * MaterialDiffuse;
>>>>>>> dev
}

void Specular(void)
{
	vec3 MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);

<<<<<<< HEAD
	vec3 lightDir = normalize(vec3(-uCamera.pos));
	vec3 viewDir = normalize(uCamera.pos - varPos);
	vec3 reflect = normalize(reflect(lightDir, Normal));
	float spec = pow(max(0.0, dot(viewDir, reflect)), 32);

	vec3 specular = MaterialSpecular * vec3(1) * spec * 0.5;

	if (SpecularMap != vec3(0))
		SpecularColor = specular * MaterialSpecular * SpecularMap;
	else
		SpecularColor = specular * MaterialSpecular;
=======
	vec3 lightDir = normalize(vec3(1, 1, 1));
	vec3 viewDir = TBN * normalize(uCamera.pos - varFragPos);
	vec3 reflectDir = reflect(-lightDir, Normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = 0.5 * spec * vec3(1, 1, 1);

	if (SpecularMap == vec3(0))
		SpecularColor = specular * MaterialSpecular * SpecularMap;
	else
		SpecularColor = specular * MaterialSpecular;

	SpecularColor = viewDir;
>>>>>>> dev
}

void Cubemap(void)
{
	vec3 I = normalize(uCamera.pos - varFragPos);
    //vec3 R = reflect(I, normalize(varFragPos));
   	vec3 R = normalize(reflect(I, Normal));
<<<<<<< HEAD
   	CubemapColor = textureCube(uReflectionMap, vec3(R.x, R.y, R.z)).rgb * MaterialUnif[13];
=======
   	CubemapColor = textureCube(uReflectionMap, vec3(R.x, R.y, R.z)).rgb * MaterialUnif[13] * 5;
>>>>>>> dev
}

void Final(void)
{
	vec4 Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);

	if (DiffuseMap != vec4(0))
	{
<<<<<<< HEAD
		if (CubemapColor != vec3(0))
=======
		if (CubemapColor == vec3(0))
>>>>>>> dev
			FinalColor = Lighting * DiffuseMap + vec4(CubemapColor, 1.0);
		else
			FinalColor = Lighting * DiffuseMap;
	}
	else
	{
		if (CubemapColor != vec3(0))
			FinalColor = Lighting + vec4(CubemapColor, 1.0);
		else
			FinalColor = Lighting;
	}

<<<<<<< HEAD
	//FinalColor = vec4(CubemapColor, 1.0);
=======
	FinalColor = vec4(SpecularColor, 1.0);
>>>>>>> dev
}



