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
	out vec3 varFragPos;
	out mat3 varTBN;

	//@Uniform uModel
	//@Uniform uView
	//@Uniform uProjection

	uniform mat4 uModel;
	uniform mat4 uView;
	uniform mat4 uProjection;

	void main()
	{
		Position = uProjection * uView * uModel * vec4(aPos, 1.0);

		vec3 normal = normalize(vec3(uModel * vec4(aNorm, 0.0)));
		vec3 tangent = normalize(vec3(uModel * vec4(aTang, 0.0)));
		vec3 bitangent = cross(normal, tangent);

		varPos = vec3(uModel * vec4(aPos, 1.0));
		varUV = aUV;
		varNormal = normal;
		varFragPos = vec3(uModel * vec4(aPos, 1.0));
		varTBN = transpose(mat3(tangent, bitangent, normal));
	}

#endif

#ifdef FragmentShader

	#define LIGHT_NUM 8

	in vec3 varPos;
	in vec2 varUV;
	in vec3 varNormal;
	in vec3 varFragPos;
	in mat3 varTBN;

	struct Material
	{
		sampler2D DiffuseMap;
		sampler2D SpecularMap;
		sampler2D NormalMap;
		sampler2D DetailDiffuseMap;
		sampler2D DetailNormalMap;
		samplerCube ReflectionMap;
		sampler2D EmissionMap;

		vec2 Tiling;
		vec2 DetailTiling;

		vec4 Color;
		vec3 AmbientColor;
		vec3 DiffuseColor;
		vec3 SpecularColor;

		float ReflectionPower;
		float EmissionStrength;
		float DetailNormalStrength;

		float Rim;
		float RimPower;
		float RimBias;
		vec3 RimColor;

		bool Lighting;
	};

	struct Camera
	{
		vec3 Position;
	};

	//@Uniform uMaterial.DiffuseMap
	//@Uniform uMaterial.SpecularMap
	//@Uniform uMaterial.NormalMap
	//@Uniform uMaterial.DetailDiffuseMap
	//@Uniform uMaterial.DetailNormalMap
	//@Uniform uMaterial.ReflectionMap
	//@Uniform uMaterial.EmissionMap
	//@Uniform uMaterial.Tiling
	//@Uniform uMaterial.DetailTiling
	//@Uniform uMaterial.Color
	//@Uniform uMaterial.AmbientColor
	//@Uniform uMaterial.DiffuseColor
	//@Uniform uMaterial.SpecularColor
	//@Uniform uMaterial.ReflectionPower
	//@Uniform uMaterial.EmissionStrength
	//@Uniform uMaterial.DetailNormalStrength
	//@Uniform uMaterial.Rim
	//@Uniform uMaterial.RimPower
	//@Uniform uMaterial.RimBias
	//@Uniform uMaterial.RimColor
	//@Uniform uMaterial.Lighting
	//@Uniform uLighting
	//@Uniform uCamera.Position

	uniform Material uMaterial;
	uniform float uLighting[15 * LIGHT_NUM];
	uniform Camera uCamera;

	vec4 DiffuseMap;
	vec3 SpecularMap;
	vec3 NormalMap;
	vec4 DetailDiffuseMap;
	vec3 DetailNormalMap;

	bool IsSpecularMap = false;

	vec3 Normal;

	vec3 AmbientColor = vec3(0);
	vec3 DiffuseColor = vec3(0);
	vec3 SpecularColor = vec3(0);
	vec3 CubemapColor = vec3(0);
	vec3 RimColor = vec3(0);
	vec4 Lighting = vec4(1);

	void Init(void);
	void LightCalc(int id);
	void RimCalc();
	void Cubemap(void);
	void Final(void);

	void main(void)
	{
		Init();

		if (uMaterial.Lighting == true)
		{
			for (int i = 0; i < LIGHT_NUM; i++)
			{
				LightCalc(i);
			}

			RimCalc();

			Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor + RimColor, 1.0);
		}
		
		if (textureSize(uMaterial.ReflectionMap, 0).x > 1)
		{
			Cubemap();
		}

		Final();
	}

	void Init(void)
	{
		DiffuseMap = texture(uMaterial.DiffuseMap, varUV * uMaterial.Tiling);
		SpecularMap = texture(uMaterial.SpecularMap, varUV * uMaterial.Tiling).rgb;
		NormalMap = texture(uMaterial.NormalMap, varUV * uMaterial.Tiling).rgb;
		DetailDiffuseMap = texture(uMaterial.DetailDiffuseMap, varUV * uMaterial.DetailTiling);
		DetailNormalMap = texture(uMaterial.DetailNormalMap, varUV * uMaterial.DetailTiling).rgb;

		if (textureSize(uMaterial.DiffuseMap, 0).x > 1 && textureSize(uMaterial.DetailDiffuseMap, 0).x > 1)
		{
			DiffuseMap = vec4(DiffuseMap.rgb * DetailDiffuseMap.rgb * 1.8f, DiffuseMap.a);
		}
		else if (textureSize(uMaterial.DetailDiffuseMap, 0).x > 1)
		{
			DiffuseMap = DetailDiffuseMap;
		}

		if (textureSize(uMaterial.NormalMap, 0).x > 1 && textureSize(uMaterial.DetailNormalMap, 0).x > 1)
		{
			NormalMap = mix(NormalMap.rgb, DetailNormalMap.rgb, uMaterial.DetailNormalStrength);
		}
		else if (textureSize(uMaterial.DetailNormalMap, 0).x > 1)
		{
			NormalMap = DetailNormalMap.rgb;
		}

		if (NormalMap != vec3(0))
			Normal = normalize(NormalMap * 2.0 - 1.0) * varTBN;
		else
			Normal = varNormal;

		if (textureSize(uMaterial.SpecularMap, 0).x > 1)
			IsSpecularMap = true;
	}

	void LightCalc(int id)
	{
		int Offset = id * 15;

		vec3 LightColor = vec3(uLighting[Offset + 0], uLighting[Offset + 1], uLighting[Offset + 2]);
		vec3 LightPos = vec3(uLighting[Offset + 3], uLighting[Offset + 4], uLighting[Offset + 5]);
		vec3 LightDir = vec3(uLighting[Offset + 6], uLighting[Offset + 7], uLighting[Offset + 8]);
		float LightType = uLighting[Offset + 9];
		float LightConstant = uLighting[Offset + 10];
		float LightLinear = uLighting[Offset + 11];
		float LightQuadratic = uLighting[Offset + 12];
		float LightInnerCutoff = uLighting[Offset + 13];
		float LightOuterCutoff = uLighting[Offset + 14];

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

		vec3 viewDir = normalize(uCamera.Position - varFragPos);

		float diff = max(0.0, dot(Normal, -lightDir));

		vec3 reflect = normalize(reflect(lightDir, Normal));
		float spec = pow(max(0.0, dot(viewDir, reflect)), 32);
		vec3 specular = uMaterial.SpecularColor * LightColor * spec * 0.5;

		vec3 tmpAmbient = vec3(0);
		vec3 tmpDiffuse = vec3(0);
		vec3 tmpSpecular = vec3(0);

		tmpAmbient = uMaterial.AmbientColor * LightColor * vec3(uMaterial.Color);
		tmpDiffuse = LightColor * uMaterial.DiffuseColor * diff * uMaterial.Color.rgb;
		
		if (IsSpecularMap)
			tmpSpecular = specular * uMaterial.SpecularColor * uMaterial.Color.rgb * SpecularMap;
		else
			tmpSpecular = specular * uMaterial.SpecularColor * uMaterial.Color.rgb;

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

		AmbientColor = uMaterial.AmbientColor;
		DiffuseColor += tmpDiffuse;
		SpecularColor += tmpSpecular;
	}

	void RimCalc()
	{
		vec3 ViewDirection = normalize(uCamera.Position - varFragPos);

		float Rim = pow(1.0 + uMaterial.RimBias - max(dot(Normal, ViewDirection), 0.0), uMaterial.RimPower);

		RimColor = Rim * uMaterial.RimColor * uMaterial.Rim;
	}

	void Cubemap(void)
	{
		vec3 I = normalize(uCamera.Position - varFragPos);
		vec3 R = normalize(reflect(I, Normal));
		CubemapColor = texture(uMaterial.ReflectionMap, -vec3(R.x, R.y, R.z)).rgb * uMaterial.ReflectionPower;

		if (IsSpecularMap)
		{
			CubemapColor *= SpecularMap;
		}

		CubemapColor *= uMaterial.Color.rgb;
	}

	void Final(void)
	{
		FragColor = vec4(0);

		if (DiffuseMap.xyz != vec3(0))
		{
			FragColor = Lighting * DiffuseMap * uMaterial.Color;
		}
		else
		{
			FragColor = Lighting * uMaterial.Color;
		}

		FragColor += vec4(CubemapColor, 0);
		FragColor += texture(uMaterial.EmissionMap, varUV) * uMaterial.EmissionStrength;

		if (texture(uMaterial.DiffuseMap, varUV).a < 0.5) discard;
	}

#endif












