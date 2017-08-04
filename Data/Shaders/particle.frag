#version 130

varying vec2 varTexCoord;

uniform vec4 uColor;
uniform sampler2D uTex;

void main()
{
	vec4 tex = texture(uTex, varTexCoord);

	if (tex.w < 0.05)
		discard;

	if (tex.xyz != vec3(0))
		gl_FragColor = uColor * tex;
	else
		gl_FragColor = uColor;

	//gl_FragColor = vec4(varTexCoord, 1.0, 1.0);
}
