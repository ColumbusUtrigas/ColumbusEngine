#ifdef VertexShader

	//@Attribute aPos  0
	//@Attribute aUV   1
	//@Attribute aNorm 2
	//@Attribute aTang 3

	in vec3 aPos;
	in vec2 aUV;
	in vec3 aNorm;
	in vec3 aTang;

	out vec3 varPos;
	out vec2 varUV;
	out vec3 varNormal;
	out mat3 varTBN;

	//@Uniform uModel
	//@Uniform uViewProjection
	//@Uniform uView
	//@Uniform uProjection

	uniform mat4 uModel;
	uniform mat4 uViewProjection;
	uniform mat4 uView;
	uniform mat4 uProjection;

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

#endif

#ifdef FragmentShader

	#define PI 3.141592653
	#define LAMBERTIAN 0.318310 // 1.0 / PI
	#define COLOR_EXP  0.454545 //1.0 / 2.2
	#define LIGHT_NUM 4

	in vec3 varPos;
	in vec2 varUV;
	in vec3 varNormal;
	in mat3 varTBN;

	struct Material
	{
		Texture2D AlbedoMap;
		Texture2D NormalMap;
		Texture2D RoughnessMap;
		Texture2D MetallicMap;
		Texture2D OcclusionMap;
		Texture2D EmissionMap;
		Texture2D DetailAlbedoMap;
		Texture2D DetailNormalMap;
		TextureCube IrradianceMap;
		TextureCube EnvironmentMap;
		Texture2D   IntegrationMap;

		bool HasAlbedoMap;
		bool HasNormalMap;
		bool HasRoughnessMap;
		bool HasMetallicMap;
		bool HasOcclusionMap;
		bool HasDetailAlbedoMap;
		bool HasDetailNormalMap;

		vec2 Tiling;
		vec2 DetailTiling;

		vec4 Albedo;
		float Roughness;
		float Metallic;
		float EmissionStrength;

		bool Transparent;
	};

	struct Camera
	{
		vec3 Position;
	};

	//@Uniform uMaterial.AlbedoMap
	//@Uniform uMaterial.NormalMap
	//@Uniform uMaterial.RoughnessMap
	//@Uniform uMaterial.MetallicMap
	//@Uniform uMaterial.OcclusionMap
	//@Uniform uMaterial.EmissionMap
	//@Uniform uMaterial.DetailAlbedoMap
	//@Uniform uMaterial.DetailNormalMap
	//@Uniform uMaterial.IrradianceMap
	//@Uniform uMaterial.EnvironmentMap
	//@Uniform uMaterial.IntegrationMap

	//@Uniform uMaterial.HasAlbedoMap
	//@Uniform uMaterial.HasNormalMap
	//@Uniform uMaterial.HasRoughnessMap
	//@Uniform uMaterial.HasMetallicMap
	//@Uniform uMaterial.HasOcclusionMap
	//@Uniform uMaterial.HasDetailAlbedoMap
	//@Uniform uMaterial.HasDetailNormalMap

	//@Uniform uMaterial.Tiling
	//@Uniform uMaterial.DetailTiling
	//@Uniform uMaterial.Albedo
	//@Uniform uMaterial.Roughness
	//@Uniform uMaterial.Metallic
	//@Uniform uMaterial.EmissionStrength
	//@Uniform uMaterial.Transparent
	//@Uniform uLighting
	//@Uniform uCamera.Position

	uniform Material uMaterial;
	uniform float uLighting[13 * LIGHT_NUM];
	uniform Camera uCamera;

	vec2 TiledUV;
	vec2 TiledDetailUV;

	vec4 Albedo;
	vec3 Normal;
	float Roughness;
	float Metallic;

	void Init(void);
	void Final(void);

	void main(void)
	{
		Init();
		Final();
	}

	vec3 NormalBlend(in vec4 n1, in vec4 n2)
	{
		//UDN
		vec3 c = vec3(2, 1, 0);
		vec3 r;
		r = n2.xyz * c.yyz + n1.xyz;
		r = r * c.xxx - c.xxy;
		return normalize(r);
	}

	void Init(void)
	{
		TiledUV = varUV * uMaterial.Tiling;
		TiledDetailUV = varUV * uMaterial.DetailTiling;

		if (uMaterial.HasAlbedoMap)
		{
			vec4 AlbedoSample = Sample2D(uMaterial.AlbedoMap, TiledUV);

			if (uMaterial.HasDetailAlbedoMap)
				Albedo = vec4(AlbedoSample.rgb * Sample2D(uMaterial.DetailAlbedoMap, TiledDetailUV).rgb * 1.8f, AlbedoSample.a);
			else
				Albedo = AlbedoSample;
		}
		else
			if (uMaterial.HasDetailAlbedoMap)
				Albedo = Sample2D(uMaterial.DetailAlbedoMap, TiledDetailUV);
			else
				Albedo = vec4(1);

		if (uMaterial.HasNormalMap)
			if (uMaterial.HasDetailNormalMap)
				Normal = NormalBlend(Sample2D(uMaterial.NormalMap, TiledUV), Sample2D(uMaterial.DetailNormalMap, TiledDetailUV)) * varTBN;
			else
				Normal = normalize(Sample2D(uMaterial.NormalMap, TiledUV).rgb * 2.0 - 1.0) * varTBN;
		else
			if (uMaterial.HasDetailNormalMap)
				Normal = normalize(Sample2D(uMaterial.DetailNormalMap, TiledDetailUV).rgb * 2.0 - 1.0) * varTBN;
			else
				Normal = varNormal;

		if (uMaterial.HasRoughnessMap)
			Roughness = Sample2D(uMaterial.RoughnessMap, TiledUV).r;
		else
			Roughness = uMaterial.Roughness;

		if (uMaterial.HasMetallicMap)
			Metallic = Sample2D(uMaterial.MetallicMap, TiledUV).r;
		else
			Metallic = uMaterial.Metallic;
	}

	vec3 LambertDiffuseBRDF(in vec3 color)
	{
		return color * LAMBERTIAN;
	}

	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a      = roughness * roughness;
		float a2     = a * a;
		float NdotH  = max(dot(N, H), 0.0);
		float NdotH2 = NdotH * NdotH;

		float num   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;

		return num / denom;
	}

	float GeometryGGX(float a, float ndotl, float ndotv)
	{
		float a2 = max(a * a, 0.001f);

		float gv = ndotl * sqrt(a2 + (1.0f - a2) * ndotv * ndotv);
		float gl = ndotv * sqrt(a2 + (1.0f - a2) * ndotl * ndotl);

		return 0.5f / max(gv + gl, 0.001f);
	}

	vec3 Fresnel(in float cosTheta, in vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
	}

	vec3 FresnelRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
	}

	vec3 CookTorranceSpecularBRDF(in vec3 N, in vec3 L, in vec3 V, in vec3 H, out vec3 F)
	{
		vec3 F0 = vec3(0.04); 
		F0 = mix(F0, vec3(1), Metallic);

		float NdotV = max(0, dot(N, V));
		float NdotL = max(0, dot(N, L));

		float D = DistributionGGX(N, H, Roughness);
		      F = FresnelRoughness(NdotV, F0, Roughness);
		float G = GeometryGGX(Roughness * Roughness, NdotL, NdotV);

		return D * F * G * LAMBERTIAN * NdotL;
	}

	vec3 LightCalc(int id, out vec3 F)
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

		float AO = uMaterial.HasOcclusionMap ? Sample2D(uMaterial.OcclusionMap, varUV * uMaterial.Tiling).r : 1.0;
		float Distance = length(LightPos - varPos);
		float Attenuation = 1.0; if (int(LightType) != 0) Attenuation = clamp(1.0 - Distance * Distance / (LightRange * LightRange), 0.0, 1.0); Attenuation *= Attenuation;

		vec3 N, L, V, H;
		N = normalize(Normal);
		L = normalize(LightPos - varPos); if (int(LightType) == 0) L = normalize(-LightDir);
		V = normalize(uCamera.Position - varPos);
		H = normalize(V + L);

		float NdotL = max(0, dot(N, L));

		vec3 DiffuseBRDF = LambertDiffuseBRDF(uMaterial.Albedo.rgb) * AO;
		vec3 SpecularBRDF = CookTorranceSpecularBRDF(N, L, V, H, F);

		float Factor = 1.0 - Metallic;

		return (Factor * DiffuseBRDF + SpecularBRDF) * NdotL * LightColor * Attenuation;
	}

	vec3 Lights()
	{
		vec3 BRDF = vec3(0);
		vec3 F = vec3(0);
		vec3 V = normalize(uCamera.Position - varPos);
		vec3 R = reflect(-V, Normal);

		BRDF += LightCalc(0, F);
		BRDF += LightCalc(1, F);
		BRDF += LightCalc(2, F);
		BRDF += LightCalc(3, F);

		const float MAX_REFLECTION_LOD = 7.0;
		vec3 prefilteredColor = SampleCubeLod(uMaterial.EnvironmentMap, R,  Roughness * MAX_REFLECTION_LOD).rgb;
		vec2 envBRDF  = Sample2D(uMaterial.IntegrationMap, vec2(max(dot(Normal, V), 0.0), Roughness)).rg;
		vec3 Specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
		vec3 Ambient = SampleCube(uMaterial.IrradianceMap, Normal).rgb;
		float AO = uMaterial.HasOcclusionMap ? Sample2D(uMaterial.OcclusionMap, varUV * uMaterial.Tiling).r : 1.0;

		BRDF += (1.0 - Metallic) * Ambient * 0.1 * AO;

		BRDF = BRDF / (BRDF + vec3(1.0));
		BRDF = pow(BRDF, vec3(COLOR_EXP));

		BRDF += Specular * AO;

		return BRDF * uMaterial.Albedo.rgb;
	}

	vec2 EncodeNormal(in vec3 n)
	{
		return vec2(n.xy / sqrt(8.0 * n.z + 8.0) + 0.5);
	}

	void Final(void)
	{
		vec4 Color = vec4(Lights(), uMaterial.Albedo.a) * Albedo;

		Color.rgb += Sample2D(uMaterial.EmissionMap, TiledUV).rgb * uMaterial.EmissionStrength;

		FragData[0] = Color;
		FragData[1] = vec4(Normal, 1);

		if (!uMaterial.Transparent && Color.a < 1.0) discard;
		if (uMaterial.Transparent && Color.a == 1.0) discard;
	}

#endif


