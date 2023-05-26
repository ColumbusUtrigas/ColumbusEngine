struct GPULight
{
	vec4 Position;
	vec4 Direction;
	vec4 Color;
	uint Type;
};

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_SPHERE 4

layout(binding = 0, set = 0) readonly buffer VertexBuffer {
	float vertices[];
} VertexBuffers[1000];

layout(binding = 0, set = 1) readonly buffer IndexBuffer {
	uint indices[];
} IndexBuffers[1000];

layout(binding = 0, set = 2) readonly buffer UvsBuffer {
	vec2 uvs[];
} UvsBuffers[1000];

layout(binding = 0, set = 3) readonly buffer NormalsBuffer {
	float normals[];
} NormalsBuffers[1000];

layout(binding = 0, set = 4) uniform sampler2D Textures[1000];

layout(binding = 0, set = 5) readonly buffer MaterialsBuffer {
	int id;
} MaterialsBuffers[1000];

layout(binding = 0, set = 6) readonly buffer LightsBuffer {
	uint Count;
	GPULight Lights[];
} GPUSceneLights;

#define NORMALBUF NormalsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].normals
