// defines location of the base SDK shader files, it's a hack until we get a proper module system

#ifndef __cplusplus
	#define HLSL 1
#endif

#define RTXGI_COORDINATE_SYSTEM RTXGI_COORDINATE_SYSTEM_LEFT
#define RTXGI_DDGI_SHADER_REFLECTION 1
#define RTXGI_DDGI_RESOURCE_MANAGEMENT 1
#define RTXGI_DDGI_BINDLESS_RESOURCES 0

#define RTXGI_DDGI_PROBE_NUM_TEXELS 8
#define RTXGI_DDGI_PROBE_NUM_INTERIOR_TEXELS 6

#define RTXGI_DDGI_BLEND_SHARED_MEMORY 0
#define RTXGI_DDGI_BLEND_SCROLL_SHARED_MEMORY 0
#define RTXGI_DDGI_DEBUG_PROBE_INDEXING 0
#define RTXGI_DDGI_DEBUG_OCTAHEDRAL_INDEXING 0

#define RTXGI_DDGI_WAVE_LANE_COUNT 32

// Descriptor set layout bindings
// 1 SRV constants structured buffer       (0)
// 1 UAV for ray data texture array        (1)
// 1 UAV probe irradiance texture array    (2)
// 1 UAV probe distance texture array      (3)
// 1 UAV probe data texture array          (4)
// 1 UAV probe variation texture array     (5)
// 1 UAV probe variation average array     (6)

#define VOLUME_CONSTS_REGISTER 0
#define VOLUME_CONSTS_SPACE 0
#define RAY_DATA_REGISTER 1
#define RAY_DATA_SPACE 0
#if RTXGI_DDGI_BLEND_RADIANCE
	#define OUTPUT_REGISTER 2
#else
	#define OUTPUT_REGISTER 3
#endif
#define OUTPUT_SPACE 0
#define PROBE_DATA_REGISTER 4
#define PROBE_DATA_SPACE 0
#define PROBE_VARIABILITY_REGISTER 5
#define PROBE_VARIABILITY_AVERAGE_REGISTER 6
#define PROBE_VARIABILITY_SPACE 0

#define TLAS_REGISTER 0
#define TLAS_SPACE 1

#if HLSL
	#define RTXGI_PUSH_CONSTS_TYPE RTXGI_PUSH_CONSTS_TYPE_SDK
#endif