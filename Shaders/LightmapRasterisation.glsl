#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "GPUScene.glsl"

struct LightmapBakingMeshVertex
{
	vec4 V1; // xyz - position, w - normal.x
	vec4 V2; // xy - normal.yz, zw - uv2.xy
};

layout(buffer_reference, std430, buffer_reference_align = 4) buffer LightmapVertexBufferPtr {
	LightmapBakingMeshVertex Vertices[];
};

layout(push_constant) uniform Params
{
	LightmapVertexBufferPtr VertexBuffer;
	IndexBufferPtr IndexBuffer;
	uint ObjectId;
} Parameters;

#ifdef VERTEX_SHADER
	layout (location = 0) out vec3 OutNormal;
	layout (location = 1) out vec2 OutUV;
	layout (location = 2) out vec3 OutWP;

	void main()
	{
		uint Index = Parameters.IndexBuffer.indices[gl_VertexIndex];
		LightmapBakingMeshVertex Vertex = Parameters.VertexBuffer.Vertices[Index];

		// TODO: model transformation
		vec3 Position = Vertex.V1.xyz;
		vec3 Normal = vec3(Vertex.V1.w, Vertex.V2.xy);
		vec2 UV = Vertex.V2.zw;

		// GPUSceneMeshCompact Mesh = GPUSceneMeshes.Meshes[Parameters.ObjectId];
		// // index is being read from index buffer by gl_VertexIndex
		// GPUScene_Vertex Vertex = GPUScene_FetchVertex(Parameters.ObjectId, gl_VertexIndex);
		// OutWP = Vertex.Position;

		vec4 ClipspacePos = vec4(UV * 2 - 1, 0, 1);

		gl_Position = ClipspacePos;
		OutNormal = Normal;
		OutUV = UV;
		OutWP = Position;
	}
#endif

#ifdef PIXEL_SHADER
	layout(location = 0) out vec3  RT0; // normal
	layout(location = 1) out vec3  RT1; // world position
	layout(location = 2) out float RT2; // validity

	layout (location = 0) in vec3 InNormal;
	layout (location = 1) in vec2 InUV;
	layout (location = 2) in vec3 InWP;

	void main()
	{
		RT0 = InNormal;
		RT1 = InWP;
		RT2 = 1;
	}
#endif