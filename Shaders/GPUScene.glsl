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

layout(buffer_reference, std430, buffer_reference_align = 4) buffer TangentsBufferPtr {
	vec4 tangentsWithSign[];
};

// TODO: separate mesh and mesh instance?
struct GPUSceneMeshCompact
{
	mat4 Transform; // 64

	VertexBufferPtr   VertexBuffer; // 72
	IndexBufferPtr    IndexBuffer;  // 80
	UvsBufferPtr      Uv1Buffer;    // 88
	UvsBufferPtr      Uv2Buffer;    // 96
	NormalsBufferPtr  NormalsBuffer; // 104
	TangentsBufferPtr TangentsBuffer; // 112

	uint VertexCount; // 116
	uint IndexCount;  // 120

	int MaterialId;  // 124
	int LightmapId; // 128
};

struct GPUSceneMaterialCompact
{
	vec4 AlbedoFactor; // 16
	vec4 EmissiveFactor; // 32

	// textures
	int AlbedoId;
	int NormalId;
	int OrmId;
	int EmissiveId; // 48

	float Roughness; // 52
	float Metallic; // 56

	int _pad, _pad2; // 64
};

struct GPUMaterialSampledData
{
	vec3 Albedo;
	vec3 Normal; // -1 to 1, tangent space
	float Roughness;
	float Metallic;
};

struct GPUViewCamera
{
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	mat4 ViewProjectionMatrix; // 192

	mat4 InverseViewMatrix;
	mat4 InverseProjectionMatrix;
	mat4 InverseViewProjectionMatrix; // 384

	vec4 CameraPosition;
	vec4 CameraForward;
	vec4 CameraUp;
	vec4 CameraRight; // 448

	vec2 Jittering; // 456

	int _pad[14]; // 512
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
	GPUViewCamera CameraCur;
	GPUViewCamera CameraPrev;
	ivec2 RenderSize;
	ivec2 OutputSize;

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

layout(binding = 4, set = GPUSCENE_SCENE_SET) uniform sampler GPUSceneSampler;

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
	vec4 TangentAndSign;
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

vec4 GPUScene_FetchVertexTangentFromMesh(GPUSceneMeshCompact Mesh, uint Index)
{
	return Mesh.TangentsBuffer.tangentsWithSign[nonuniformEXT(Index)];
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

GPUScene_Vertex GPUScene_FetchVertexByIndex(uint ObjectId, uint Index)
{
	GPUSceneMeshCompact mesh = GPUSceneMeshes.Meshes[nonuniformEXT(ObjectId)];

	GPUScene_Vertex vertex;
	vertex.Position = GPUScene_FetchVertexPositionFromMesh(mesh, Index);
	vertex.Normal = GPUScene_FetchVertexNormalFromMesh(mesh, Index);
	vertex.TangentAndSign = GPUScene_FetchVertexTangentFromMesh(mesh, Index);
	vertex.UV = GPUScene_FetchVertexUVFromMesh(mesh, Index);
	vertex.UV2 = GPUScene_FetchVertexUV2FromMesh(mesh, Index);
	vertex.MaterialId = GPUScene_FetchVertexMaterialFromMesh(mesh, Index);
	return vertex;
}

// reads index by VertexIndex
GPUScene_Vertex GPUScene_FetchVertex(uint ObjectId, uint VertexIndex)
{
	uint index = GPUScene_FetchIndex(ObjectId, VertexIndex);
	return GPUScene_FetchVertexByIndex(ObjectId, index);
}

vec4 SampleTextureWithDefault(int TextureId, vec2 UV, vec4 Default)
{
	if (TextureId != -1)
	{
		return textureLod(Textures[nonuniformEXT(TextureId)], UV, 0.0f);
	}
	else
	{
		return Default;
	}
}

GPUMaterialSampledData GPUScene_SampleMaterial(uint MaterialId, vec2 UV)
{
	GPUMaterialSampledData Result;

	if (MaterialId != -1)
	{
		GPUSceneMaterialCompact Material = GPUSceneMaterials.Materials[nonuniformEXT(MaterialId)];

		// TODO: proper LOD selection

		Result.Albedo = SampleTextureWithDefault(Material.AlbedoId, UV, vec4(1)).rgb * Material.AlbedoFactor.rgb;
		Result.Normal = SampleTextureWithDefault(Material.NormalId, UV, vec4(0, 0, 1, 0)).rgb;

		vec3 ORM = SampleTextureWithDefault(Material.OrmId, UV, vec4(1, Material.Roughness, Material.Metallic, 1)).rgb;

		Result.Roughness = ORM.g;
		Result.Metallic = ORM.b;
	}
	else
	{
		Result.Albedo = vec3(1);
		Result.Normal = vec3(0, 0, 1);
		Result.Roughness = 1;
		Result.Metallic = 0;
	}

	return Result;
}

#endif // GPUSCENE_GLSL