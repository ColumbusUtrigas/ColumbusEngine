#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "FSR1.glsl"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
	AU4 Const0,Const1,Const2,Const3;
	SetupFSR(Const0,Const1,Const2,Const3);

	// Do remapping of local xy in workgroup for a more PS-like swizzle pattern.
	AU2 gxy = ARmp8x8(gl_LocalInvocationID.x) + AU2(gl_WorkGroupID.x << 4u, gl_WorkGroupID.y << 4u);
	AH3 c;

	c = CurrFilter(gxy,Const0,Const1,Const2,Const3);
	imageStore(OutputTexture, ASU2(gxy), AH4(c, 1));
	gxy.x += 8u;
	c = CurrFilter(gxy,Const0,Const1,Const2,Const3);
	imageStore(OutputTexture, ASU2(gxy), AH4(c, 1));
	gxy.y += 8u;
	c = CurrFilter(gxy,Const0,Const1,Const2,Const3);
	imageStore(OutputTexture, ASU2(gxy), AH4(c, 1));
	gxy.x -= 8u;
	c = CurrFilter(gxy,Const0,Const1,Const2,Const3);
	imageStore(OutputTexture, ASU2(gxy), AH4(c, 1));
}