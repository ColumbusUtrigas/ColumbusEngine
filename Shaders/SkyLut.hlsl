#include "SH.hlsli"
#include "Common.hlsli"
#include "SkyCommon.hlsli"

#define GPU_SCENE_NO_BINDINGS
#include "GPUScene.hlsli"

[[vk::binding(0, 0)]] RWStructuredBuffer<GPUSceneStruct> GPUSceneScene;

groupshared sh2 SH[8][8][3];

#define ShRLocal SH[dtid.x][dtid.y][0]
#define ShGLocal SH[dtid.x][dtid.y][1]
#define ShBLocal SH[dtid.x][dtid.y][2]

[numthreads(8, 8, 1)]
void main(uint3 dtid : SV_DispatchThreadID)
{
    // initialise groupshared
    SH[dtid.x][dtid.y][0] = shZero();
    SH[dtid.x][dtid.y][1] = shZero();
    SH[dtid.x][dtid.y][2] = shZero();

    GroupMemoryBarrierWithGroupSync();
    
    uint rng = dtid.x + dtid.y*8; // initial seed
    
    const int SamplesPerThread = 8;

    const float3 CameraPosition = GPUSceneScene[0].CameraCur.CameraPosition.xyz;
    const float3 SunDirection = GPUSceneScene[0].SunDirection.xyz;
    
    for (int i = 0; i < SamplesPerThread; i++)
    {
        float3 SampleRayDirection = Random::RandomDirectionSphere(Random::UniformDistrubition2d(rng));
        float3 Sample = Sky::Atmosphere(CameraPosition, SampleRayDirection, SunDirection, GPUSceneScene[0].Sky);
        
        // update SH
        sh2 sh = shEvaluate(SampleRayDirection);
        ShRLocal = shHanningConvolution(shAdd(ShRLocal, shScale(sh, Sample.r)), 0.1f);
        ShGLocal = shHanningConvolution(shAdd(ShGLocal, shScale(sh, Sample.g)), 0.1f);
        ShBLocal = shHanningConvolution(shAdd(ShBLocal, shScale(sh, Sample.b)), 0.1f);
    }
    
    const float shFactor = 4.0 * shPI / (float) (SamplesPerThread * 8 * 8);
    ShRLocal = shScale(ShRLocal, shFactor);
    ShGLocal = shScale(ShGLocal, shFactor);
    ShBLocal = shScale(ShBLocal, shFactor);
    
    GroupMemoryBarrierWithGroupSync();
    
    // only one threads writes to global memory
    if (dtid.x == 0 && dtid.y == 0)
    {
        sh2 ShR = shZero();
        sh2 ShG = shZero();
        sh2 ShB = shZero();
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                ShR = shAdd(ShR, SH[i][j][0]);
                ShG = shAdd(ShG, SH[i][j][1]);
                ShB = shAdd(ShB, SH[i][j][2]);
            }
        }
        
        GPUSceneScene[0].SkySHR = ShR;
        GPUSceneScene[0].SkySHG = ShG;
        GPUSceneScene[0].SkySHB = ShB;
    }
}