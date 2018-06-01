#ifdef VertexShader

	in vec3 aPos;
	in vec2 aUV;

	out vec2 UV;

	void main(void)
	{
		Position = vec4(aPos, 1.0);
		UV = aUV;
	}

#endif

#ifdef FragmentShader

	uniform sampler2D uColor;
	uniform sampler2D uDepth;
	uniform vec2 uResolution;

	in vec2 UV;

	float blurSizeH = 1.0 / uResolution.x;
	float blurSizeV = 1.0 / uResolution.y;

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
	    vec2 pixel = vec2(uResolution.y / uResolution.x, 1.0) * radius * 0.025;
	    float r = 1.0;

		for (float j = 0.0; j < GOLDEN_ANGLE * ITERATIONS; j += GOLDEN_ANGLE)
	    {
	       	
			vec3 col = texture2D(uColor, UV + pixel * Sample(j, r)).xyz;
			vec3 bokeh = vec3(0.5) + pow(col, vec3(10.0)) * amount;
			acc += col * bokeh;
			div += bokeh;
		}
		return acc / div;
	}

	void main()
	{
		float d = pow(texture(uDepth, UV).x, 256);

		//FragColor = vec4(BokehBlur(d * 0.3, 6), 1.0);
		FragColor = vec4(texture(uColor, UV).rgb, 1.0);
	}

#endif








