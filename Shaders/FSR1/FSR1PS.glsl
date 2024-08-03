#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "FSR1.glsl"

#ifdef VERTEX_SHADER
const vec2 pos[3] = {
	vec2(-1, -1),
	vec2(-1, 3),
	vec2(3, -1),
};

void main()
{
	gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
}
#endif

#ifdef PIXEL_SHADER
layout(location = 0) out vec4 RT0;

void main()
{
	AU4 Const0,Const1,Const2,Const3;
	SetupFSR(Const0,Const1,Const2,Const3);

	AU2 gxy = AU2(gl_FragCoord.xy);
	RT0 = vec4(CurrFilter(gxy,Const0,Const1,Const2,Const3), 1);
}
#endif