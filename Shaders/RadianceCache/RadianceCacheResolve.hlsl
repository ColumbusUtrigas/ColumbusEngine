#define RADIANCE_CACHE_SET 0
#define SHARC_UPDATE 0
#define SHARC_QUERY 0
#include "RadianceCache.hlsli"

[numthreads(256, 1, 1)]
void main(uint3 DispatchId : SV_DispatchThreadID)
{
	SharcResolveParameters Resolve;
	Resolve.cameraPositionPrev  = RadianceCacheConfig.PreviousCameraPosition.xyz;
	Resolve.accumulationFrameNum = RadianceCacheConfig.Config.z;
	Resolve.responsiveFrameNum  = 1;
	Resolve.staleFrameNumMax    = RadianceCacheConfig.Config.w;
	Resolve.frameIndex          = RadianceCacheConfig.Config.y;
	SharcResolveEntry(DispatchId.x, GetRadianceCacheParameters(), Resolve);
}
