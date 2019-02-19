#ifdef VertexShader

	//@Attribute aPos 0
	in vec3 aPos;
	out vec3 Pos;

	//@Uniform Projection
	//@Uniform View
	uniform mat4 Projection;
	uniform mat4 View;

	void main()
	{
		gl_Position = Projection * View * vec4(aPos, 1);
		Pos = aPos;
	}

#endif

#ifdef FragmentShader

	in vec3 Pos;

	//@Uniform Roughness
	//@Uniform EnvironmentMap
	uniform float Roughness;
	uniform TextureCube EnvironmentMap;

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
				PrefilteredColor += SampleCube(EnvironmentMap, L).rgb * NdotL;
				TotalWeight      += NdotL;
			}
		}

		FragColor = vec4(PrefilteredColor / TotalWeight, 1.0);
	}

#endif


