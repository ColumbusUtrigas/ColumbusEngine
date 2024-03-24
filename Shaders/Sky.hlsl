// TODO: changeable colour (very easy)

// TODO: bottom-center-top gradient (easy)

// TODO: sky texture (easy)

// TODO: unify computations with path tracing (medium, requires full HLSL for PT)

// TODO: atmosphere scattering (hard)
// https://sebh.github.io/publications/egsr2020.pdf
// https://www.shadertoy.com/view/wllyW4
// https://github.com/sebh/UnrealEngineSkyAtmosphere/blob/master/Resources/RenderSkyCommon.hlsl
// https://www.shadertoy.com/view/slSXRW
// https://www.scratchapixel.com/lessons/procedural-generation-virtual-worlds/simulating-sky/simulating-colors-of-the-sky.html

struct VS_TO_PS
{
	float4 Pos : SV_POSITION;
};

static const float2 pos[3] = {
	float2(-1, -1),
	float2(-1, +3),
	float2(+3, -1)
};

VS_TO_PS Vertex(uint VertexID : SV_VertexID)
{
    VS_TO_PS Out;
    Out.Pos = float4(pos[VertexID], 0.9999, 1);
    return Out;
}

float4 Pixel(VS_TO_PS In) : SV_TARGET
{
    return float4(0.412, 0.796, 1.0, 1);
}