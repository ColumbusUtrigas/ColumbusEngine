namespace Columbus
{

	const char* gScreenSpaceVertexShader = 
	R"(
	#version 130
	in vec2 Pos;
	in vec2 UV;

	out vec2 Texcoord;

	void main(void)
	{
		gl_Position = vec4(Pos, 0, 1);
		Texcoord = UV;
	}
	)";

	const char* gScreenSpaceFragmentShader =
	R"(
	#version 130
	out vec4 FragColor;
	in vec2 Texcoord;

	uniform sampler2D BaseTexture;

	void main(void)
	{
		FragColor = texture(BaseTexture, Texcoord);
	}
	)";

	const char* gTonemapFragmentShader = 
	R"(
	#version 130
	out vec4 FragColor;
	in vec2 Texcoord;

	uniform sampler2D BaseTexture;
	uniform float Exposure;
	uniform float Gamma;

	void main(void)
	{
		vec3 HDR = texture(BaseTexture, Texcoord).rgb;
		vec3 Mapped = vec3(1.0) - exp(-HDR * Exposure);
		Mapped = pow(Mapped, vec3(1.0 / Gamma));

		FragColor = vec4(Mapped, 1.0);
	}
	)";

	const char* gGaussBlurFragmentShader = 
	R"(
	#version 130

	out vec4 FragColor;

	uniform sampler2D BaseTexture;
	uniform bool Horizontal;
	uniform float Radius;

	in vec2 Texcoord;

	uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

	void main()
	{
		vec2 tex_offset = 1.0 / textureSize(BaseTexture, 0) * Radius;
		vec3 result = texture(BaseTexture, Texcoord).rgb * weight[0];

		if (Horizontal)
		{
			for (int i = 1; i < 5; ++i)
			{
				result += texture(BaseTexture, Texcoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
				result += texture(BaseTexture, Texcoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
			}
		}
		else
		{
			for (int i = 1; i < 5; ++i)
			{
				result += texture(BaseTexture, Texcoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
				result += texture(BaseTexture, Texcoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
			}
		}

		FragColor = vec4(result, 1.0);
	}
	)";

	const char* gBloomBrightFragmentShader =
	R"(
	#version 130

	out vec4 FragColor;

	uniform sampler2D BaseTexture;
	uniform float Treshold;

	in vec2 Texcoord;

	float luma(vec3 color)
	{
		return dot(color, vec3(0.299, 0.587, 0.114));
	}

	void main(void)
	{
		vec4 color = texture(BaseTexture, Texcoord);
		FragColor = luma(color.rgb) > Treshold ? color : vec4(0, 0, 0, 1);
	}
	)";

	const char* gBloomFragmentShader = 
	R"(
	#version 130

	out vec4 FragColor;

	uniform sampler2D BaseTexture;
	uniform sampler2D Blur;
	uniform float Intensity;

	in vec2 Texcoord;

	void main()
	{
		vec3 hdrColor = texture(BaseTexture, Texcoord).rgb;
		vec3 bloomColor = texture(Blur, Texcoord).rgb * Intensity;
		hdrColor += bloomColor;
		FragColor = vec4(hdrColor, 1.0);
	}
	)";

	const char* gFXAAFragmentShader = 
	R"(
	#version 130

	out vec4 FragColor;

	uniform sampler2D BaseTexture;
	uniform vec2 Resolution;

	#define FXAA_SPAN_MAX 8.0
	#define FXAA_REDUCE_MUL   (1.0/FXAA_SPAN_MAX)
	#define FXAA_REDUCE_MIN   (1.0/128.0)
	#define FXAA_SUBPIX_SHIFT (1.0/4.0)

	vec4 fxaa(sampler2D tex, vec2 fragCoord, vec2 resolution,
		vec2 v_rgbNW, vec2 v_rgbNE, 
		vec2 v_rgbSW, vec2 v_rgbSE, 
		vec2 v_rgbM)
	{
		vec4 color;
		mediump vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
		vec3 rgbNW = texture2D(tex, v_rgbNW).xyz;
		vec3 rgbNE = texture2D(tex, v_rgbNE).xyz;
		vec3 rgbSW = texture2D(tex, v_rgbSW).xyz;
		vec3 rgbSE = texture2D(tex, v_rgbSE).xyz;
		vec4 texColor = texture2D(tex, v_rgbM);
		vec3 rgbM  = texColor.xyz;
		vec3 luma = vec3(0.299, 0.587, 0.114);
		float lumaNW = dot(rgbNW, luma);
		float lumaNE = dot(rgbNE, luma);
		float lumaSW = dot(rgbSW, luma);
		float lumaSE = dot(rgbSE, luma);
		float lumaM  = dot(rgbM,  luma);
		float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
		float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

		mediump vec2 dir;
		dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
		dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

		float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
		          (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

		float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
		dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
		max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
		dir * rcpDirMin)) * inverseVP;

		vec3 rgbA = 0.5 * (
		texture2D(tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
		texture2D(tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
		vec3 rgbB = rgbA * 0.5 + 0.25 * (
		texture2D(tex, fragCoord * inverseVP + dir * -0.5).xyz +
		texture2D(tex, fragCoord * inverseVP + dir * 0.5).xyz);

		float lumaB = dot(rgbB, luma);
		if ((lumaB < lumaMin) || (lumaB > lumaMax))
		color = vec4(rgbA, texColor.a);
		else
		color = vec4(rgbB, texColor.a);
		return color;
	}

	void texcoords(vec2 fragCoord, vec2 resolution,
			out vec2 v_rgbNW, out vec2 v_rgbNE,
			out vec2 v_rgbSW, out vec2 v_rgbSE,
			out vec2 v_rgbM)
	{
		vec2 inverseVP = 1.0 / resolution.xy;
		v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
		v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
		v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
		v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
		v_rgbM = vec2(fragCoord * inverseVP);
	}

	void main(void)
	{
		vec2 v_rgbNW;
		vec2 v_rgbNE;
		vec2 v_rgbSW;
		vec2 v_rgbSE;
		vec2 v_rgbM;

		texcoords(gl_FragCoord.xy, Resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
		FragColor =  fxaa(BaseTexture, gl_FragCoord.xy, Resolution, v_rgbNW, v_rgbNE, v_rgbSW, v_rgbSE, v_rgbM);
	}
	)";

	const char* gIconVertexShader =
	R"(
	#version 130

	in vec2 Position;
	in vec2 UV;

	uniform vec2 Pos;
	uniform vec2 Size;

	out vec2 Texcoord;

	void main(void)
	{
		gl_Position = vec4(Position * Size + Pos, 0, 1);
		Texcoord = UV;
	}
	)";

	const char* gIconFragmentShader =
	R"(
	#version 130

	out vec4 FragColor;

	uniform sampler2D Texture;

	in vec2 Texcoord;

	void main(void)
	{
		FragColor = texture(Texture, Texcoord);
	}
	)";

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
		//FragColor = vec4(pow(textureCube(Skybox, Texcoord).rgb, vec3(1.5)), 1);
		FragColor = texture(Skybox, Texcoord);
		gl_FragDepth = 0x7FFFFFFF;
	}
	)";

	const char* gSkyboxCubemapGenerationVertexShader =
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

	const char* gSkyboxCubemapGenerationFragmentShader =
	R"(
	#version 130
	out vec4 FragColor;
	in vec3 Pos;

	uniform sampler2D BaseMap;

	const vec2 invAtan = vec2(0.1591, 0.3183);
	vec2 SampleSphericalMap(vec3 v)
	{
		vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
		uv *= invAtan;
		uv += 0.5;
		return uv;
	}

	void main()
	{		
		vec2 uv = SampleSphericalMap(normalize(Pos)); 
		vec3 color = texture(BaseMap, uv).rgb;

		FragColor = vec4(color, 1.0);
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

	in vec2 Position;
	in vec2 Texcoord;

	void main()
	{
		gl_Position = vec4(Position, 0, 1);
		TexCoords = Texcoord;
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


