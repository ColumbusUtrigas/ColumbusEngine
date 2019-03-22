#shader vertex

out vec2 UV;

void main(void)
{
	Position = ftransform();
	UV = gl_MultiTexCoord0.xy;
}

#shader fragment

#uniform Texture2D BaseTexture
#uniform float Exposure
#uniform float Gamma

in vec2 UV;

void main()
{
	vec3 HDR = texture(BaseTexture, UV).rgb;
	vec3 Mapped = vec3(1.0) - exp(-HDR * Exposure);
	Mapped = pow(Mapped, vec3(1.0 / Gamma));

	FragColor = vec4(Mapped, 1.0);
}


