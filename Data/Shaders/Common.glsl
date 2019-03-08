#define PI 3.141592653
#define COLOR_EXP  0.454545 //1.0 / 2.2

vec3 NormalBlend(vec4 n1, vec4 n2)
{
	//UDN
	vec3 c = vec3(2, 1, 0);
	vec3 r;
	r = n2.xyz * c.yyz + n1.xyz;
	r = r * c.xxx - c.xxy;
	return normalize(r);
}

vec2 ExtractSubUV(int FrameNumber, vec2 Texcoords, vec2 Frame)
{
	int FrameHorizontal = FrameNumber % int(Frame.x);
	int FrameVertical = FrameNumber / int(Frame.x) - 1;
	vec2 Begin = 1.0 / Frame;

	return vec2(Texcoords.x * Begin.x + Begin.x * FrameHorizontal,
	            Texcoords.y * Begin.y + Begin.y * FrameVertical);
}


