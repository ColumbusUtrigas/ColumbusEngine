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

vec3 AmbientColor = vec3(0);
vec3 DiffuseColor = vec3(0);
vec3 SpecularColor = vec3(0);

void Light();

void main()
{
	vec4 tex = texture(uTex, varTexCoord);
	vec4 Gradient;

	if (varIsGradient != 0.0)
		Gradient = mix(uStartColor, uFinalColor, varTime / varTTL);

	Light();
	vec4 Lighting = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);

	if (varIsGradient == 0.0)
	{
		if (tex.xyz != vec3(0))
			gl_FragColor = uColor * tex * Lighting;
		else
			gl_FragColor = uColor * Lighting;
	} else
	{
		gl_FragColor = Gradient * tex * Lighting;
	}

	if (uDiscard == 1 && gl_FragColor.w < 0.05)
			discard;
}

void Light()
{
	vec4 MaterialColor = vec4(MaterialUnif[0], MaterialUnif[1], MaterialUnif[2], MaterialUnif[3]);
	vec3 MaterialAmbient = vec3(MaterialUnif[4], MaterialUnif[5], MaterialUnif[6]);
	vec3 MaterialDiffuse = vec3(MaterialUnif[7], MaterialUnif[8], MaterialUnif[9]);
	vec3 MaterialSpecular = vec3(MaterialUnif[10], MaterialUnif[11], MaterialUnif[12]);

	float distance = length(vec3(0, 0, 5) - varPos);
	float attenuation = 1.0 / (1.0 +
						0.09 * distance +
						0.032 * (distance * distance));

	AmbientColor = MaterialAmbient * vec3(1) * vec3(MaterialColor) * attenuation;
	DiffuseColor = vec3(1) * MaterialDiffuse * MaterialColor.xyz * attenuation;
}




