#version 130

uniform sampler2D uColor;
uniform sampler2D uDepth;

out vec4 FinalColor;

vec2 UV = gl_TexCoord[0].xy;

vec3 GaussianBlur(vec2 size)
{
	vec4 sum = vec4(0.0);

	//float sX = blurSizeH * size.x;
	//float sY = blurSizeV * size.y;

	float sX = 1.0 / 640  * size.x;
	float sY = 1.0 / 480  * size.y;

    for (int x = -4; x <= 4; x++)
    {
        for (int y = -4; y <= 4; y++)
        {
            sum += texture(uColor, vec2(UV.x + x * sX, UV.y + y * sY)) / 81.0;
        }
    }

    return sum.xyz;
}

void main()
{
	float d = pow(texture(uDepth, UV).x, 256);

	FinalColor = vec4(d, d, d, 1.0);
	FinalColor = vec4(texture(uColor, UV).rgb, 1.0);
	FinalColor = vec4(GaussianBlur(vec2(d, d)), 1.0);
}
