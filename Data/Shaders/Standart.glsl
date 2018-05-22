#ifdef VertexShader

	in vec3 aPos;
	in vec2 aUV;
	in vec3 aNorm;
	in vec3 aTang;

	out vec3 varPos;
	out vec2 varUV;
	out vec3 varNormal;
	out vec3 varFragPos;
	out mat3 varTBN;

	uniform mat4 uModel;
	uniform mat4 uView;
	uniform mat4 uProjection;

	void main()
	{
		Position = uProjection * uView * uModel * vec4(aPos, 1.0);
		//Position = vec4(aPos, 1.0) * uModel * uView * uProjection;
		//Position = uProjection * uView * vec4(aPos, 1.0) * uModel;

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
		samplerCube ReflectionMap;

		vec4 Color;
		vec3 AmbientColor;
		vec3 DiffuseColor;
		vec3 SpecularColor;
		float ReflectionPower;
		bool Lighting;
	};

	struct Camera
	{
		vec3 Position;
	};

	uniform Material uMaterial;
	uniform float uLighting[15 * LIGHT_NUM];
	uniform Camera uCamera;

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
	void LightCalc(int id);
	void Cubemap(void);
	void Final(void);

	void main(void)
	{
		Init();

		if (uMaterial.Lighting == true)
		{
			LightCalc(0);
			LightCalc(1);
			LightCalc(2);
			LightCalc(3);
			LightCalc(4);
			LightCalc(5);
			LightCalc(6);
			LightCalc(7);

			Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);
		}
		
		Cubemap();

		Final();
	}

	void Init(void)
	{
		DiffuseMap = texture(uMaterial.DiffuseMap, varUV);
		SpecularMap = vec3(texture(uMaterial.SpecularMap, varUV));
		NormalMap = vec3(texture(uMaterial.NormalMap, varUV));

		if (DiffuseMap.w <= 0.1) discard;

		TBN = varTBN;

		if (textureSize(uMaterial.SpecularMap, 1).xy != vec2(0))
			IsSpecularMap = true;

		if (NormalMap != vec3(0))
			Normal = normalize(NormalMap * 2.0 - 1.0) * TBN;
		else
			Normal = varNormal;
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

		AmbientColor += tmpAmbient;
		DiffuseColor += tmpDiffuse;
		SpecularColor += tmpSpecular;
	}

	void Cubemap(void)
	{
		vec3 I = normalize(uCamera.Position - varFragPos);
	    //vec3 R = reflect(I, normalize(varFragPos));
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

#endif












