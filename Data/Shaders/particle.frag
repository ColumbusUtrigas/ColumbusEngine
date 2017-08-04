#version 130

varying vec2 varTexCoord;
varying float varTime;
varying float varTTL;
varying float varIsGradient;

uniform vec4 uColor;
uniform sampler2D uTex;

void main()
{
	vec4 tex = texture(uTex, varTexCoord);
	vec4 Gradient;

	if (varIsGradient != 0.0)
		Gradient = mix(vec4(1, 1, 1, 0.5), vec4(1, 1, 1, 0.1), varTime / varTTL);

	if (varIsGradient == 0.0)
	{
		if (tex.xyz != vec3(0))
			gl_FragColor = uColor * tex;
		else
			gl_FragColor = uColor;
	}

	
	if (varIsGradient != 0.0)
	{
		gl_FragColor = Gradient * tex;
	}
	if (gl_FragColor.w < 0.1)
		discard;
}
