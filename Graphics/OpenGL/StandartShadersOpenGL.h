namespace Columbus
{

	const char* gSkyboxVertexShader = 
	R"(
	#version 130
	in vec3 Pos;

	out vec3 Texcoord;

	uniform mat4 ViewProjection;

	void main(void)
	{
		gl_Position = ViewProjection * vec4(Pos, 1.0);
		Texcoord = Pos;
	}
	)";

	const char* gSkyboxFragmentShader = 
	R"(
	#version 130
	out vec4 FragColor;
	in vec3 Texcoord;

	uniform samplerCube Skybox;

	void main(void)
	{
		FragColor = textureCube(Skybox, Texcoord);
		gl_FragDepth = 0x7FFFFFFF;
	}
	)";

	const char* gIrradianceGenerationVertexShader = 
	R"(
	#version 130
	in vec3 Position;
	out vec3 Pos;

	uniform mat4 Projection;
	uniform mat4 View;

	void main()
	{
		gl_Position = Projection * View * vec4(Position, 1);
		Pos = Position;
	}
	)";

	const char* gIrradianceGenerationFragmentShader =
	R"(
	#version 130
	out vec4 FragColor;

	in vec3 Pos;

	uniform samplerCube EnvironmentMap;

	const float PI = 3.14159265359;

	void main()
	{
		vec3 Normal = normalize(Pos);  
		vec3 Irradiance = vec3(0.0);

		vec3 Up    = vec3(0.0, 1.0, 0.0);
		vec3 Right = cross(Up, Normal);
		Up         = cross(Normal, Right);

		float SampleDelta = 0.025;
		float NumSamples = 0.0;

		for(float Phi = 0.0; Phi < 2.0 * PI; Phi += SampleDelta)
		{
			for(float Theta = 0.0; Theta < 0.5 * PI; Theta += SampleDelta)
			{
				vec3 TangentSample = vec3(sin(Theta) * cos(Phi), sin(Theta) * sin(Phi), cos(Theta));
				vec3 SampleVec = TangentSample.x * Right + TangentSample.y * Up + TangentSample.z * Normal; 

				Irradiance += textureCube(EnvironmentMap, SampleVec).rgb * cos(Theta) * sin(Theta);
				NumSamples += 1.0;
			}
		}

		FragColor = vec4(PI * Irradiance * (1.0 / NumSamples), 1.0);
	}
	)";

	const char* gPrefilterGenerationVertexShader = 
	R"(
	#version 130
	in vec3 Position;
	out vec3 Pos;

	uniform mat4 Projection;
	uniform mat4 View;

	void main()
	{
		gl_Position = Projection * View * vec4(Position, 1);
		Pos = Position;
	}
	)";

	const char* gPrefilterGenerationFragmentShader = 
	R"(
	#version 130
	out vec4 FragColor;
	in vec3 Pos;

	uniform float Roughness;
	uniform samplerCube EnvironmentMap;

	const float PI = 3.14159265359;

	float RadicalInverse(uint n, uint base)
	{
		float InvBase = 1.0 / float(base);
		float Denom   = 1.0;
		float Result  = 0.0;

		for(uint i = 0u; i < 32u; ++i)
		{
			if(n > 0u)
			{
				Denom   = mod(float(n), 2.0);
				Result += Denom * InvBase;
				InvBase = InvBase / 2.0;
				n       = uint(float(n) / 2.0);
			}
		}

		return Result;
	}

	vec2 Hammersley(uint i, uint N)
	{
		return vec2(float(i) / float(N), RadicalInverse(i, 2u));
	}

	vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness * roughness;

		float Phi = 2.0 * PI * Xi.x;
		float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
		float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

		vec3 H;
		H.x = cos(Phi) * SinTheta;
		H.y = sin(Phi) * SinTheta;
		H.z = CosTheta;

		vec3 Up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 Tangent   = normalize(cross(Up, N));
		vec3 Bitangent = cross(N, Tangent);

		return normalize(Tangent * H.x + Bitangent * H.y + N * H.z);
	}

	void main()
	{		
		vec3 N = normalize(Pos);
		vec3 R = N;
		vec3 V = R;

		const uint SAMPLE_COUNT = 1024u;
		float TotalWeight = 0.0;   
		vec3 PrefilteredColor = vec3(0.0);

		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H  = ImportanceSampleGGX(Xi, N, Roughness);
			vec3 L  = normalize(2.0 * dot(V, H) * H - V);

			float NdotL = max(dot(N, L), 0.0);

			if(NdotL > 0.0)
			{
				PrefilteredColor += textureCube(EnvironmentMap, L).rgb * NdotL;
				TotalWeight      += NdotL;
			}
		}

		FragColor = vec4(PrefilteredColor / TotalWeight, 1.0);
	}
	)";

	const char* gIntegrationGenerationVertexShader = 
	R"(
	#version 130
	out vec2 TexCoords;

	void main()
	{
		gl_Position = ftransform();
		TexCoords = gl_MultiTexCoord0.xy;
	}
	)";

	const char* gIntegrationGenerationFragmentShader = 
	R"(
	#version 130
	out vec4 FragColor;
	in vec2 TexCoords;

	const float PI = 3.14159265359;

	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float a = roughness;
		float k = (a * a) / 2.0;

		float Nom   = NdotV;
		float Denom = NdotV * (1.0 - k) + k;

		return Nom / Denom;
	}

	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}

	vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness * roughness;

		float Phi = 2.0 * PI * Xi.x;
		float CosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
		float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

		vec3 H;
		H.x = cos(Phi) * SinTheta;
		H.y = sin(Phi) * SinTheta;
		H.z = CosTheta;

		vec3 Up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 Tangent   = normalize(cross(Up, N));
		vec3 Bitangent = cross(N, Tangent);

		return normalize(Tangent * H.x + Bitangent * H.y + N * H.z);
	}

	float RadicalInverse(uint n, uint base)
	{
		float InvBase = 1.0 / float(base);
		float Denom   = 1.0;
		float Result  = 0.0;

		for(uint i = 0u; i < 32u; ++i)
		{
			if(n > 0u)
			{
				Denom   = mod(float(n), 2.0);
				Result += Denom * InvBase;
				InvBase = InvBase / 2.0;
				n       = uint(float(n) / 2.0);
			}
		}

		return Result;
	}

	vec2 Hammersley(uint i, uint N)
	{
	    return vec2(float(i) / float(N), RadicalInverse(i, 2u));
	}

	vec2 IntegrateBRDF(float NdotV, float roughness)
	{
		vec3 V;
		V.x = sqrt(1.0 - NdotV * NdotV);
		V.y = 0.0;
		V.z = NdotV;

		float A = 0.0;
		float B = 0.0;

		vec3 N = vec3(0.0, 0.0, 1.0);

		const uint SAMPLE_COUNT = 1024u;

		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
			vec3 L  = normalize(2.0 * dot(V, H) * H - V);

			float NdotL = max(L.z, 0.0);
			float NdotH = max(H.z, 0.0);
			float VdotH = max(dot(V, H), 0.0);

			if(NdotL > 0.0)
			{
				float G = GeometrySmith(N, V, L, roughness);
				float G_Vis = (G * VdotH) / (NdotH * NdotV);
				float Fc = pow(1.0 - VdotH, 5.0);

				A += (1.0 - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}

		return vec2(A, B) / float(SAMPLE_COUNT);
	}

	void main() 
	{
		vec2 IntegratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
		FragColor = vec4(IntegratedBRDF, 1, 1);
	}
	)";

}


