layout(binding = 0, set = 0) uniform sampler2D Input;

#ifdef COMPUTE_SHADER
layout(binding = 1, set = 0, rgba32f) uniform image2D OutputTexture;
#endif

#define A_GPU 1
#define A_GLSL 1
#define A_HALF 1
#include "../Shared/FSR1/ffx_a.h"
#define FSR_EASU_H 1
#define FSR_RCAS_H 1
#include "../Shared/FSR1/ffx_fsr1.h"

layout(push_constant) uniform Params
{
	ivec2 ImageSize;
	ivec2 MaxImageSize;
	ivec2 OutputSize;
	float Sharpening;
	bool IsSharpeningPass; // 32 bit bool
	bool IsHdr;
} Parameters;

AH4 SampleInputTexture(AH4 c)
{
	return c;

	// TODO?
	// AH3 c2 = c.rgb;
	// if (Parameters.IsHdr)
	// 	FsrSrtmH(c2); // tonemap to 0-1 range
	// return AH4(c2, 1);
}

//declare input callbacks
AH4 FsrEasuRH(AF2 p){return SampleInputTexture(AH4(textureGather(Input,p,0)));}
AH4 FsrEasuGH(AF2 p){return SampleInputTexture(AH4(textureGather(Input,p,1)));}
AH4 FsrEasuBH(AF2 p){return SampleInputTexture(AH4(textureGather(Input,p,2)));}

AH4 FsrRcasLoadH(ASW2 p) { return SampleInputTexture(AH4(texelFetch(Input, ASU2(p), 0))); }
void FsrRcasInputH(inout AH1 r,inout AH1 g,inout AH1 b){}

#define SAMPLE_BILINEAR 0
#define SAMPLE_EASU 1
#define SAMPLE_RCAS 0
#define SAMPLE_SLOW_FALLBACK 0

void SetupFSR(out AU4 Const0, out AU4 Const1, out AU4 Const2, out AU4 Const3)
{
	ivec2 ImageSize = Parameters.ImageSize;
	ivec2 MaxImageSize = Parameters.MaxImageSize;
	ivec2 OutputSize = Parameters.OutputSize;

	if (Parameters.IsSharpeningPass)
	{
		FsrRcasCon(Const0, Parameters.Sharpening);
	}
	else
	{
		FsrEasuCon(Const0,Const1,Const2,Const3,
			ImageSize.x, ImageSize.y,
			// MaxImageSize.x, MaxImageSize.y, // for dynamic resolution, TODO?
			ImageSize.x, ImageSize.y,
			OutputSize.x, OutputSize.y);
	}
}

AH3 CurrFilter(AU2 pos, AU4 Const0, AU4 Const1, AU4 Const2, AU4 Const3)
{
	if (Parameters.IsSharpeningPass)
	{
		AH3 c;
		FsrRcasH(c.r, c.g, c.b, pos, Const0);

		// TODO: Sample.x = (hdr && !bUseRcas) ? 1 : 0;
		if(Parameters.IsHdr)
		{
			// FsrSrtmInvH(c);
			// c *= c;
		}

		return c;
	}
	else
	{
		AH3 c;
		FsrEasuH(c, pos, Const0, Const1, Const2, Const3);

		if(Parameters.IsHdr)
		{
			// FsrSrtmInvH(c);
			// c *= c;
		}

		// if( Sample.x == 1 ) // TODO:
		// 	c *= c;
		// imageStore(OutputTexture, ASU2(pos), AH4(c, 1));
		return c;
	}
}
