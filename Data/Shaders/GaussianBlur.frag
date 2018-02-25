uniform sampler2D uFrame;
uniform vec2 uWindowSize;
uniform vec2 uBlurSize;

void main(void)
{
	vec2 UV = gl_TexCoord[0].xy;

	float blurSizeH = 1.0 / uWindowSize.x;
	float blurSizeV = 1.0 / uWindowSize.y;

	vec4 sum = vec4(0.0);

	float sX = blurSizeH * uBlurSize.x;
	float sY = blurSizeV * uBlurSize.y;

    for (int x = -4; x <= 4; x++)
    {
        for (int y = -4; y <= 4; y++)
        {
            sum += texture(uFrame, vec2(UV.x + x * sX, UV.y + y * sY)) / 81.0;
        }
    }

    FragColor = vec4(sum.xyz, 1.0);
}




