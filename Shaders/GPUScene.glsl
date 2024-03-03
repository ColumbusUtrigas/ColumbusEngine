#ifndef GPUSCENE_GLSL
#define GPUSCENE_GLSL

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

	int MaterialId;  // 116
	int LightmapId; // 120

	int _pad[2]; // 128
};

struct GPUSceneMaterialCompact
{
	int AlbedoId;

	float Roughness;
	float Metallic;

	int _pad; // 16
};

struct GPUMaterialSampledData
{
	vec3 Albedo;
	float Roughness;
	float Metallic;
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

layout(binding = 0, set = GPUSCENE_TEXTURES_SET) uniform sampler2D Textures[2000];

layout(binding = 0, set = GPUSCENE_SCENE_SET) readonly buffer GPUSceneBuffer {
	uint MeshesCount;
	uint MaterialsCount;
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

layout(binding = 3, set = GPUSCENE_SCENE_SET) readonly buffer MaterialsBuffer {
	GPUSceneMaterialCompact Materials[];
} GPUSceneMaterials;

uint GPUScene_GetMeshesCount()
{
	return GPUSceneScene.MeshesCount;
}

uint GPUScene_GetMaterialsCount()
{
	return GPUSceneScene.MaterialsCount;
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
	vec2 UV2;
	uint MaterialId;
};

uint GPUScene_FetchIndex(uint ObjectId, uint Index)
{
	return GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)].IndexBuffer.indices[nonuniformEXT(Index)];
}

vec3 GPUScene_FetchVertexPositionFromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	vec3 vertex = vec3(0);
	vertex.x = Mesh.VertexBuffer.vertices[nonuniformEXT(Index * 3 + 0)];
	vertex.y = Mesh.VertexBuffer.vertices[nonuniformEXT(Index * 3 + 1)];
	vertex.z = Mesh.VertexBuffer.vertices[nonuniformEXT(Index * 3 + 2)];
	return vertex;
}

vec3 GPUScene_FetchVertexNormalFromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	vec3 normal = vec3(0);
	normal.x = Mesh.NormalsBuffer.normals[nonuniformEXT(Index * 3 + 0)];
	normal.y = Mesh.NormalsBuffer.normals[nonuniformEXT(Index * 3 + 1)];
	normal.z = Mesh.NormalsBuffer.normals[nonuniformEXT(Index * 3 + 2)];
	return normal;
}

vec2 GPUScene_FetchVertexUVFromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	return Mesh.Uv1Buffer.uvs[nonuniformEXT(Index)];
}

vec2 GPUScene_FetchVertexUV2FromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	// TODO: check if buffer reference is 0 instead
	if (Mesh.LightmapId != -1)
	{
		return Mesh.Uv2Buffer.uvs[nonuniformEXT(Index)];
	}

	return vec2(0);
}

uint GPUScene_FetchVertexMaterialFromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	// TODO: per-vertex materials
	return Mesh.MaterialId;
}

GPUScene_Vertex GPUScene_FetchVertex(uint ObjectId, uint VertexIndex)
{
	uint index = GPUScene_FetchIndex(ObjectId, VertexIndex);
	GPUSceneMeshCompact mesh = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)];

	GPUScene_Vertex vertex;
	vertex.Position = GPUScene_FetchVertexPositionFromMesh(mesh, index);
	vertex.Normal = GPUScene_FetchVertexNormalFromMesh(mesh, index);
	vertex.UV = GPUScene_FetchVertexUVFromMesh(mesh, index);
	vertex.UV2 = GPUScene_FetchVertexUV2FromMesh(mesh, index);
	vertex.MaterialId = GPUScene_FetchVertexMaterialFromMesh(mesh, index);
	return vertex;
}

GPUMaterialSampledData GPUScene_SampleMaterial(uint MaterialId, vec2 UV)
{
	GPUMaterialSampledData Result;

	if (MaterialId != -1)
	{
		GPUSceneMaterialCompact Material = GPUSceneMaterials.Materials[nonuniformEXT(MaterialId)];

		if (Material.AlbedoId != -1)
		{
			Result.Albedo = textureLod(Textures[nonuniformEXT(Material.AlbedoId)], UV, 0.0f).rgb;
		}
		else
		{
			Result.Albedo = vec3(1);
		}

		Result.Roughness = Material.Roughness;
		Result.Metallic = Material.Metallic;
	}
	else
	{
		Result.Albedo = vec3(1);
		Result.Roughness = 1;
		Result.Metallic = 0;
	}

	return Result;
}

#endif // GPUSCENE_GLSL