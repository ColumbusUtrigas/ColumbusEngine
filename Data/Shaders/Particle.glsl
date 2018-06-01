#ifdef VertexShader

	in vec3 aPos;
	in vec2 aUV;
	in vec3 aPoses;
	in vec4 aTimes;
	in vec4 aColors;
	in vec3 aSizes;

	out vec3 varPos;
	out vec2 varTexCoord;
	out vec4 varColor;

	uniform mat4 uView;
	uniform mat4 uProjection;

	uniform vec2 uSubUV;
	uniform float uBillboard;

	void RotationMatrix(vec3 axis, float angle, out mat4 Result)
	{
	    axis = normalize(axis);
	    float s = sin(angle);
	    float c = cos(angle);
	    float oc = 1.0 - c;
	    
	    Result =  mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	                   oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	                   oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	                   0.0,                                0.0,                                0.0,                                1.0);
	}


	void main(void)
	{
		mat4 Rotation;
		mat4 ModelView = uView * mat4(1);
		RotationMatrix(vec3(0, 0, 1), aTimes.z * 0.011111, Rotation);

		ModelView[0][0] = 1;
		ModelView[0][1] = 0;
		ModelView[0][2] = 0;

		ModelView[1][0] = 0;
		ModelView[1][1] = 1;
		ModelView[1][2] = 0;

		ModelView[2][0] = 0;
		ModelView[2][1] = 0;
		ModelView[2][2] = 1;

		vec3 pos = aPoses;

		int frameNumber = int(aTimes.w);

		int frameHorizontal = frameNumber % int(uSubUV.x);
		int frameVertical = int(uSubUV.y) - int(frameNumber / uSubUV.x) - 1;

		float frame_X = 1.0 / uSubUV.x;
		float frame_Y = 1.0 / uSubUV.y;

		vec2 frame = vec2(aUV.x * frame_X + frame_X * frameHorizontal,
		                  aUV.y * frame_Y + frame_Y * frameVertical);

		if (uBillboard != 0.0)
		{
			Position = uProjection * (uView * vec4(pos, 1.0) + vec4(aPos, 0.0) * vec4(aSizes, 0.0) * Rotation);
		} else
		{
			Position = uProjection * uView * (vec4(pos, 1.0) + vec4(aPos * aSizes, 0.0) * Rotation);
		}

		varPos = pos + aPos;
		varTexCoord = frame;
		varColor = aColors;
	}

#endif

#ifdef FragmentShader

	#define MAX_LIGHTS_NUMBER 4

	in vec3 varPos;
	in vec2 varTexCoord;
	in vec4 varColor;

	uniform sampler2D uTex;
	uniform int uDiscard;

	uniform float MaterialUnif[15];
	uniform float LightUnif[120];

	vec3 AmbientColor = vec3(0);
	vec3 DiffuseColor = vec3(0);
	vec3 SpecularColor = vec3(0);

	void Light(int id);

	void main()
	{
		vec4 tex = texture(uTex, varTexCoord);
		vec4 Color = varColor;
		vec4 Lighting = vec4(1);

		if (textureSize(uTex, 0).x >= 1)
			Color = varColor * tex;
		else
			Color = varColor;

		if (MaterialUnif[14] != 0.0)
		{
			for (int i = 0; i < MAX_LIGHTS_NUMBER; i++)
			{
				Light(i);
			}

			Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);
		}

		FragColor = Color * Lighting;

		if (FragColor.w < 0.01) discard;
	}

	void Light(int id)
	{
		int offset = id * 15;

		float LightType = LightUnif[9 + offset];

		vec3 LightColor = vec3(LightUnif[0 + offset], LightUnif[1 + offset], LightUnif[2 + offset]);
		vec3 LightPos = vec3(LightUnif[3 + offset], LightUnif[4 + offset], LightUnif[5 + offset]);
		vec3 LightDir = vec3(LightUnif[6 + offset], LightUnif[7 + offset], LightUnif[8 + offset]);

		float LightConstant = LightUnif[10 + offset];
		float LightLinear = LightUnif[11 + offset];
		float LightQuadratic = LightUnif[12 + offset];
		float LightInnerAngle = LightUnif[13 + offset];
		float LightOuterAngle = LightUnif[14 + offset];

		vec4 MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
		vec3 MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
		vec3 MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);
		vec3 MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);

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

		vec3 Normal = normalize(-LightPos);

		float diff = max(0.0, dot(Normal, -lightDir));

		vec3 tmpAmbient = vec3(0);
		vec3 tmpDiffuse = vec3(0);
		vec3 tmpSpecular = vec3(0);

		tmpAmbient = MaterialAmbient * LightColor * vec3(MaterialColor);
		tmpDiffuse = LightColor * MaterialDiffuse * diff * MaterialColor.xyz;

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

#endif









