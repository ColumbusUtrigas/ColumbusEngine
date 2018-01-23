#version 130

varying vec3 varPos;
varying vec2 varTexCoord;
varying float varTime;
varying float varTTL;
varying float varIsGradient;

uniform vec4 uColor;
uniform sampler2D uTex;
uniform vec4 uStartColor;
uniform vec4 uFinalColor;
uniform int uDiscard;

uniform float MaterialUnif[14];
uniform float LightUnif[120];

vec3 AmbientColor = vec3(0);
vec3 DiffuseColor = vec3(0);
vec3 SpecularColor = vec3(0);
bool IsLightEnabled = false;

void Light(int id);

void main()
{
	vec4 tex = texture(uTex, varTexCoord);
	vec4 Gradient = mix(uStartColor, uFinalColor, varTime / varTTL) * clamp(varIsGradient, 0.0, 1.0);
	vec4 Color = uColor;

	if (varIsGradient == 0.0)
	{
		if (textureSize(uTex, 0).x > 1)
			Color = uColor * tex;	
	} else
	{
		if (textureSize(uTex, 0).x > 1)
			Color = uColor * tex * Gradient;
		else
			Color = uColor * Gradient;
	}

	Light(0);
	Light(1);
	Light(2);
	Light(3);
	//Light(4);
	//Light(5);
	//Light(6);
	//Light(7);

	vec4 Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);
	if (IsLightEnabled == false)
		Lighting = vec4(1);

	gl_FragColor = Color * Lighting;

	//if (uDiscard == 1 && gl_FragColor.w < 0.05)
			//discard;

	if (gl_FragColor.w < 0.01) discard;
}

void Light(int id)
{
	int offset = id * 15;

	float LightType = LightUnif[9 + offset];

	if (LightType == -1) return;
	else IsLightEnabled = true;

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




