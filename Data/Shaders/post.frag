#version 130

varying vec2 varUV;

uniform sampler2D uFrame;
uniform vec2 uWindowSize;

out vec3 FinalColor;

float blurSizeH = 1.0 / uWindowSize.x;
float blurSizeV = 1.0 / uWindowSize.y;

vec2 UV = gl_TexCoord[0].xy;
vec3 frame = texture(uFrame, UV).xyz;

vec3 Negative(void);
vec3 GaussianBlur(vec2 size);
vec3 Sobel(void);
vec3 Scanline(float scale);
vec3 Pixelization(vec2 size);

void main(void)
{
	//FinalColor = Negative();
	//FinalColor = GaussianBlur(vec2(2, 2));
	//FinalColor = Sobel();
	//FinalColor = Scanline(10);
	//FinalColor = Pixelization(vec2(10, 10));
	FinalColor = frame;
}

vec3 Negative(void)
{
	return (vec3(1) - frame);
}

vec3 GaussianBlur(vec2 size)
{
	vec4 sum = vec4(0.0);

	float sX = blurSizeH * size.x;
	float sY = blurSizeV * size.y;

    for (int x = -4; x <= 4; x++)
    {
        for (int y = -4; y <= 4; y++)
        {
            sum += texture(uFrame, vec2(UV.x + x * sX, UV.y + y * sY)) / 81.0;
        }
    }

    return sum.xyz;
}

vec3 Sobel(void)
{
	vec4 top         = texture(uFrame, vec2(UV.x, gl_TexCoord[0].y + blurSizeV));
	vec4 bottom      = texture(uFrame, vec2(UV.x, gl_TexCoord[0].y - blurSizeV));
	vec4 left        = texture(uFrame, vec2(UV.x - blurSizeH, UV.y));
	vec4 right       = texture(uFrame, vec2(UV.x + blurSizeH, UV.y));
	vec4 topLeft     = texture(uFrame, vec2(UV.x - blurSizeH, UV.y + blurSizeV));
	vec4 topRight    = texture(uFrame, vec2(UV.x + blurSizeH, UV.y + blurSizeV));
	vec4 bottomLeft  = texture(uFrame, vec2(UV.x - blurSizeH, UV.y - blurSizeV));
	vec4 bottomRight = texture(uFrame, vec2(UV.x + blurSizeH, UV.y - blurSizeV));
	vec4 sx = -topLeft - 2 * left - bottomLeft + topRight   + 2 * right  + bottomRight;
	vec4 sy = -topLeft - 2 * top  - topRight   + bottomLeft + 2 * bottom + bottomRight;
	vec4 sobel = sqrt(sx * sx + sy * sy);

	return sobel.xyz;
}

vec3 Scanline(float scale)
{
	if (mod(floor(UV.y * uWindowSize.y / scale), 2.0) == 0.0)
	    return vec3(0);
	else
	    return texture2D(uFrame, UV).xyz;
}

vec3 Pixelization(vec2 size)
{
    float dx = size.x * (1.0 / uWindowSize.x);
    float dy = size.y * (1.0 / uWindowSize.y);
    vec2 Coord = vec2(dx * floor(UV.x / dx),
                      dy * floor(UV.y / dy));
    return texture(uFrame, Coord).xyz;
}




