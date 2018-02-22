uniform sampler2D uColor;
uniform sampler2D uDepth;
uniform vec2 uResolution;

in vec2 UV;

float blurSizeH = 1.0 / uResolution.x;
float blurSizeV = 1.0 / uResolution.y;

vec3 GaussianBlur(vec2 size)
{
	vec4 sum = vec4(0.0);

	float sX = blurSizeH * size.x;
	float sY = blurSizeV * size.y;

    for (int x = -4; x <= 4; x++)
    {
        for (int y = -4; y <= 4; y++)
        {
            sum += texture(uColor, vec2(UV.x + x * sX, UV.y + y * sY)) / 81.0;
        }
    }

    return sum.xyz;
}

vec2 Sample(in float theta, inout float r)
{
    r += 1.0 / r;
	return (r-1.0) * vec2(cos(theta), sin(theta)) * .06;
}

#define ITERATIONS 150.0
#define ONEOVER_ITR  1.0 / ITERATIONS
#define PI 3.141596
#define GOLDEN_ANGLE 2.39996323

vec3 BokehBlur(float radius, float amount)
{
	vec3 acc = vec3(0.0);
	vec3 div = vec3(0.0);
    vec2 pixel = vec2(uResolution.y / uResolution.x, 1.0) * radius * .025;
    float r = 1.0;

	for (float j = 0.0; j < GOLDEN_ANGLE * ITERATIONS; j += GOLDEN_ANGLE)
    {
       	
		vec3 col = texture2D(uColor, UV + pixel * Sample(j, r)).xyz;
		vec3 bokeh = vec3(.5) + pow(col, vec3(10.0)) * amount;
		acc += col * bokeh;
		div += bokeh;
	}
	return acc / div;
}

void main()
{
	float d = pow(texture(uDepth, UV).x, 256);

	//FragColor = vec4(d, d, d, 1.0);
	//FragColor = vec4(texture(uColor, UV).rgb, 1.0);
	//FragColor = vec4(GaussianBlur(vec2(d, d)), 1.0);
	FragColor = vec4(BokehBlur(1, 10), 1.0);
    //FragColor = vec4(vec3(1) - texture(uColor, UV).rgb, 1.0);
    //FragColor = vec4(vec3(1.0) - GaussianBlur(vec2(d, d)), 1.0);
}
