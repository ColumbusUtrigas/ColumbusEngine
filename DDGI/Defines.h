// defines shared constants both for CPU and GPU

#ifndef DDGI_IMPL_DEFINES
#define DDGI_IMPL_DEFINES

#ifndef __cplusplus
	#define HLSL 1
#endif

#define RTXGI_COORDINATE_SYSTEM RTXGI_COORDINATE_SYSTEM_LEFT
#define RTXGI_DDGI_SHADER_REFLECTION 1
#define RTXGI_DDGI_RESOURCE_MANAGEMENT 1
#define RTXGI_DDGI_BINDLESS_RESOURCES 0

// bindings declaration
#define RTXGI_PUSH_CONSTS_TYPE RTXGI_PUSH_CONSTS_TYPE_SDK
#define VOLUME_CONSTS_REGISTER 0
#define VOLUME_CONSTS_SPACE 0

#define RAY_DATA_REGISTER 1
#define RAY_DATA_SPACE 0

#define PROBE_DATA_REGISTER 2
#define PROBE_DATA_SPACE 0

#define TLAS_REGISTER 0
#define TLAS_SPACE 1

#if HLSL
	uint GetDDGIVolumeIndex()
	{
		// TODO: more volumes
		return 0;
	}
#endif

#endif // DDGI_IMPL_DEFINES