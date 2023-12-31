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

struct GPUSceneMeshCompact
{
	mat4 Transform;

	uint VertexCount;
	uint IndexCount;

	// int MaterialId; // TODO:
	int TextureId;
	int LightmapId;
};

// TODO:
// struct GPUDecal;

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_SPHERE 4

// TODO: pack data more efficiently, it uses too many descriptor sets

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

#define GPUSCENE_SCENE_SET 5

layout(binding = 0, set = GPUSCENE_SCENE_SET) readonly buffer GPUSceneBuffer {
	uint MeshesCount;
	uint TexturesCount;
	uint LightsCount;
	uint DecalsCount;
} GPUSceneScene;

// TODO: decals

layout(binding = 1, set = GPUSCENE_SCENE_SET) readonly buffer LightsBuffer {
	GPULight Lights[];
} GPUSceneLights;

layout(binding = 2, set = GPUSCENE_SCENE_SET) readonly buffer MeshesBuffer {
	GPUSceneMeshCompact Meshes[];
} GPUSceneMeshes;

#define NORMALBUF NormalsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].normals

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
	return IndexBuffers[ObjectId].indices[Index];
}

vec3 GPUScene_FetchVertexPosition(uint ObjectId, uint Index)
{
	vec3 vertex = vec3(0);
	vertex.x = VertexBuffers[ObjectId].vertices[Index * 3 + 0];
	vertex.y = VertexBuffers[ObjectId].vertices[Index * 3 + 1];
	vertex.z = VertexBuffers[ObjectId].vertices[Index * 3 + 2];
	return vertex;
}

vec3 GPUScene_FetchVertexNormal(uint ObjectId, uint Index)
{
	vec3 normal = vec3(0);
	normal.x = NormalsBuffers[ObjectId].normals[Index * 3 + 0];
	normal.y = NormalsBuffers[ObjectId].normals[Index * 3 + 1];
	normal.z = NormalsBuffers[ObjectId].normals[Index * 3 + 2];
	return normal;
}

vec2 GPUScene_FetchVertexUV(uint ObjectId, uint Index)
{
	return UvsBuffers[ObjectId].uvs[Index];
}

uint GPUScene_FetchVertexMaterial(uint ObjectId, uint Index)
{
	// TODO: per-vertex materials
	return GPUSceneMeshes.Meshes[ObjectId].TextureId;
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