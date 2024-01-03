#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

struct GPULight
{
	vec4 Position;
	vec4 Direction;
	vec4 Color;
	uint Type;
	float Range;
	float SourceRadius;
	uint _pad; // 64
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer VertexBufferPtr {
	float vertices[];
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer IndexBufferPtr {
	uint indices[];
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer UvsBufferPtr {
	vec2 uvs[];
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer NormalsBufferPtr {
	float normals[];
};

struct GPUSceneMeshCompact
{
	mat4 Transform; // 64

	VertexBufferPtr  VertexBuffer; // 72
	IndexBufferPtr   IndexBuffer;  // 80
	UvsBufferPtr     Uv1Buffer;    // 88
	UvsBufferPtr     Uv2Buffer;    // 96
	NormalsBufferPtr NormalsBuffer; // 104

	uint VertexCount; // 108
	uint IndexCount;  // 112

	// int MaterialId; // TODO:
	int TextureId;  // 116
	int LightmapId; // 120

	int _pad[2]; // 128
};

// TODO:
// struct GPUDecal;

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_SPHERE 4

#define GPUSCENE_TEXTURES_SET 0
#define GPUSCENE_SCENE_SET 1

layout(binding = 0, set = GPUSCENE_TEXTURES_SET) uniform sampler2D Textures[1000];

layout(binding = 0, set = GPUSCENE_SCENE_SET) readonly buffer GPUSceneBuffer {
	uint MeshesCount;
	uint TexturesCount;
	uint LightsCount;
	uint DecalsCount;
} GPUSceneScene;

layout(binding = 1, set = GPUSCENE_SCENE_SET) readonly buffer LightsBuffer {
	GPULight Lights[];
} GPUSceneLights;

layout(binding = 2, set = GPUSCENE_SCENE_SET) readonly buffer MeshesBuffer {
	GPUSceneMeshCompact Meshes[];
} GPUSceneMeshes;

uint GPUScene_GetMeshesCount()
{
	return GPUSceneScene.MeshesCount;
}

uint GPUScene_GetTexturesCount()
{
	return GPUSceneScene.TexturesCount;
}

uint GPUScene_GetLightsCount()
{
	return GPUSceneScene.LightsCount;
}

uint GPUScene_GetDecalsCount()
{
	return GPUSceneScene.DecalsCount;
}

struct GPUScene_Vertex
{
	vec3 Position;
	vec3 Normal;
	vec2 UV;
	uint MaterialId; // TODO:
};

uint GPUScene_FetchIndex(uint ObjectId, uint Index)
{
	return GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].IndexBuffer.indices[nonuniformEXT(Index)];
}

vec3 GPUScene_FetchVertexPosition(uint ObjectId, uint Index)
{
	vec3 vertex = vec3(0);
	vertex.x = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].VertexBuffer.vertices[nonuniformEXT(Index * 3 + 0)];
	vertex.y = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].VertexBuffer.vertices[nonuniformEXT(Index * 3 + 1)];
	vertex.z = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].VertexBuffer.vertices[nonuniformEXT(Index * 3 + 2)];
	return vertex;
}

vec3 GPUScene_FetchVertexNormal(uint ObjectId, uint Index)
{
	vec3 normal = vec3(0);
	normal.x = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].NormalsBuffer.normals[nonuniformEXT(Index * 3 + 0)];
	normal.y = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].NormalsBuffer.normals[nonuniformEXT(Index * 3 + 1)];
	normal.z = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].NormalsBuffer.normals[nonuniformEXT(Index * 3 + 2)];
	return normal;
}

vec2 GPUScene_FetchVertexUV(uint ObjectId, uint Index)
{
	return GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].Uv1Buffer.uvs[nonuniformEXT(Index)];
}

uint GPUScene_FetchVertexMaterial(uint ObjectId, uint Index)
{
	// TODO: per-vertex materials
	return GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].TextureId;
}

GPUScene_Vertex GPUScene_FetchVertex(uint ObjectId, uint VertexIndex)
{
	uint index = GPUScene_FetchIndex(ObjectId, VertexIndex);

	GPUScene_Vertex vertex;
	vertex.Position = GPUScene_FetchVertexPosition(ObjectId, index);
	vertex.Normal = GPUScene_FetchVertexNormal(ObjectId, index);
	vertex.UV = GPUScene_FetchVertexUV(ObjectId, index);
	vertex.MaterialId = GPUScene_FetchVertexMaterial(ObjectId, index);
	return vertex;
}