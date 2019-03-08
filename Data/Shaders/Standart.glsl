#include "Data/Shaders/Common.glsl"

#shader vertex

#attribute vec3 aPos  0
#attribute vec2 aUV   1
#attribute vec3 aNorm 2
#attribute vec3 aTang 3

#uniform mat4 uModel
#uniform mat4 uViewProjection

out vec3 varPos;
out vec2 varUV;
out vec3 varNormal;
out mat3 varTBN;

void main()
{
	vec3 Normal = normalize(vec3(vec4(aNorm, 0.0) * uModel));
	vec3 Tangent = normalize(vec3(vec4(aTang, 0.0) * uModel));
	vec3 Bitangent = cross(Normal, Tangent);

	varPos = vec3(vec4(aPos, 1.0) * uModel);
	varUV = aUV;
	varNormal = Normal;
	varTBN = transpose(mat3(Tangent, Bitangent, Normal));

	Position = uViewProjection * vec4(varPos, 1);
}

#shader fragment

#include "Data/Shaders/BRDF.glsl"

#define LIGHT_NUM 4

in vec3 varPos;
in vec2 varUV;
in vec3 varNormal;
in mat3 varTBN;

#uniform Texture2D   AlbedoMap
#uniform Texture2D   NormalMap
#uniform Texture2D   RoughnessMap
#uniform Texture2D   MetallicMap
#uniform Texture2D   OcclusionMap
#uniform Texture2D   EmissionMap
#uniform Texture2D   DetailAlbedoMap
#uniform Texture2D   DetailNormalMap
#uniform TextureCube IrradianceMap
#uniform TextureCube EnvironmentMap
#uniform Texture2D   IntegrationMap

#uniform bool HasAlbedoMap
#uniform bool HasNormalMap
#uniform bool HasRoughnessMap
#uniform bool HasMetallicMap
#uniform bool HasOcclusionMap
#uniform bool HasDetailAlbedoMap
#uniform bool HasDetailNormalMap

#uniform vec2 Tiling
#uniform vec2 DetailTiling

#uniform vec3 uCameraPosition

#uniform vec4  Albedo
#uniform float Roughness
#uniform float Metallic
#uniform float EmissionStrength
#uniform bool  Transparent

//@Uniform uLighting

uniform float uLighting[13 * LIGHT_NUM];

vec2 _TiledUV;
vec2 _TiledDetailUV;

vec4 _Albedo;
vec3 _Normal;
float _Roughness;
float _Metallic;

void Init(void);
void Final(void);

void main(void)
{
	Init();
	Final();
}

void Init(void)
{
	_TiledUV = varUV * Tiling;
	_TiledDetailUV = varUV * DetailTiling;

	if (HasAlbedoMap)
	{
		vec4 AlbedoSample = Sample2D(AlbedoMap, _TiledUV);

		if (HasDetailAlbedoMap)
			_Albedo = vec4(AlbedoSample.rgb * Sample2D(DetailAlbedoMap, _TiledDetailUV).rgb * 1.8f, AlbedoSample.a);
		else
			_Albedo = AlbedoSample;
	}
	else
		if (HasDetailAlbedoMap)
			_Albedo = Sample2D(DetailAlbedoMap, _TiledDetailUV);
		else
			_Albedo = vec4(1);

	if (HasNormalMap)
		if (HasDetailNormalMap)
			_Normal = NormalBlend(Sample2D(NormalMap, _TiledUV), Sample2D(DetailNormalMap, _TiledDetailUV)) * varTBN;
		else
			_Normal = normalize(Sample2D(NormalMap, _TiledUV).rgb * 2.0 - 1.0) * varTBN;
	else
		if (HasDetailNormalMap)
			_Normal = normalize(Sample2D(DetailNormalMap, _TiledDetailUV).rgb * 2.0 - 1.0) * varTBN;
		else
			_Normal = varNormal;

	if (HasRoughnessMap)
		_Roughness = Sample2D(RoughnessMap, _TiledUV).r;
	else
		_Roughness = Roughness;

	if (HasMetallicMap)
		_Metallic = Sample2D(MetallicMap, _TiledUV).r;
	else
		_Metallic = Metallic;
}

vec3 LightCalc(int id, vec3 F, vec3 N, vec3 V, float NdotV)
{
	int Offset = id * 13;

	vec3 LightColor = vec3(uLighting[Offset + 0], uLighting[Offset + 1], uLighting[Offset + 2]);
	vec3 LightPos = vec3(uLighting[Offset + 3], uLighting[Offset + 4], uLighting[Offset + 5]);
	vec3 LightDir = vec3(uLighting[Offset + 6], uLighting[Offset + 7], uLighting[Offset + 8]);
	float LightType = uLighting[Offset + 9];
	float LightRange = uLighting[Offset + 10];
	//float LightInnerCutoff = uLighting[Offset + 11];
	//float LightOuterCutoff = uLighting[Offset + 12];

	if (LightColor == vec3(0)) return vec3(0);

	float AO = HasOcclusionMap ? Sample2D(OcclusionMap, _TiledUV).r : 1.0;
	float Distance = length(LightPos - varPos);
	float Attenuation = 1.0; if (int(LightType) != 0) Attenuation = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0); Attenuation *= Attenuation;

	vec3 L = normalize(LightPos - varPos); if (int(LightType) == 0) L = normalize(-LightDir);
	vec3 H = normalize(V + L);

	float NdotL = max(0, dot(N, L));

	vec3 DiffuseBRDF = LambertDiffuseBRDF(Albedo.rgb) * AO;
	vec3 SpecularBRDF = CookTorranceSpecularBRDF(N, L, H, F, NdotV, _Roughness);

	float Factor = 1.0 - _Metallic;

	return (Factor * DiffuseBRDF + SpecularBRDF) * NdotL * LightColor * Attenuation;
}

vec3 Lights(void)
{
	vec3 N = normalize(_Normal);
	vec3 V = normalize(uCameraPosition - varPos);
	vec3 R = reflect(-V, _Normal);
	float NdotV = max(0.0, dot(N, V));

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, vec3(1), _Metallic);

	vec3 BRDF = vec3(0.0);
	vec3 F = FresnelRoughness(NdotV, F0, _Roughness);

	BRDF += LightCalc(0, F, N, V, NdotV);
	BRDF += LightCalc(1, F, N, V, NdotV);
	BRDF += LightCalc(2, F, N, V, NdotV);
	BRDF += LightCalc(3, F, N, V, NdotV);

	const float MAX_REFLECTION_LOD = 7.0; // TODO
	vec3 prefilteredColor = SampleCubeLod(EnvironmentMap, R,  _Roughness * MAX_REFLECTION_LOD).rgb;
	vec2 envBRDF  = Sample2D(IntegrationMap, vec2(max(dot(_Normal, V), 0.0), _Roughness)).rg;
	vec3 Specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
	vec3 Ambient = SampleCube(IrradianceMap, _Normal).rgb;
	float AO = HasOcclusionMap ? Sample2D(OcclusionMap, _TiledUV).r : 1.0;

	BRDF += (1.0 - _Metallic) * Ambient * 0.1 * AO;

	BRDF = BRDF / (BRDF + vec3(1.0));
	BRDF = pow(BRDF, vec3(COLOR_EXP));

	BRDF += Specular * AO;

	return BRDF * Albedo.rgb;
}

vec2 EncodeNormal(in vec3 n)
{
	return vec2(n.xy / sqrt(8.0 * n.z + 8.0) + 0.5);
}

void Final(void)
{
	vec4 Color = vec4(Lights(), Albedo.a) * _Albedo;

	Color.rgb += Sample2D(EmissionMap, _TiledUV).rgb * EmissionStrength;

	FragData[0] = Color;
	FragData[1] = vec4(_Normal, 1);

	if (!Transparent && Color.a < 1.0) discard;
	if (Transparent && Color.a == 1.0) discard;
}


