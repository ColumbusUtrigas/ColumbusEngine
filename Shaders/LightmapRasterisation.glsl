#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "GPUScene.glsl"

layout(push_constant) uniform Params
{
	uint ObjectId;
} Parameters;

#ifdef VERTEX_SHADER
	layout (location = 0) out vec3 OutNormal;
	layout (location = 1) out vec2 OutUV;
	layout (location = 2) out uint OutTextureId;
	layout (location = 3) out vec3 OutWP;

	void main()
	{
		GPUSceneMeshCompact Mesh = GPUSceneMeshes.Meshes[Parameters.ObjectId];

		// index is being read from index buffer by gl_VertexIndex
		GPUScene_Vertex Vertex = GPUScene_FetchVertex(Parameters.ObjectId, gl_VertexIndex);

		OutWP = Vertex.Position;

		vec4 ClipspacePos = vec4(Vertex.UV * 2 - 1, 0, 1);

		gl_Position = ClipspacePos;
		OutNormal = Vertex.Normal;
		OutUV = Vertex.UV;
		OutTextureId = Mesh.TextureId;
	}
#endif

#ifdef PIXEL_SHADER
	layout(location = 0) out vec3  RT0; // diffuse
	layout(location = 1) out vec3  RT1; // normal
	layout(location = 2) out vec3  RT2; // world position
	layout(location = 3) out vec2  RT3; // roughness/metallic
	layout(location = 4) out float RT4; // validity

	layout (location = 0) in vec3 InNormal;
	layout (location = 1) in vec2 InUV;
	layout (location = 2) in flat uint InTextureId;
	layout (location = 3) in vec3 InWP;

	void main()
	{
		RT0 = textureLod(Textures[InTextureId], InUV, 0.0f).rgb;
		RT1 = InNormal;
		RT2 = InWP;
		RT3 = vec2(1, 0);
		RT4 = 1;
	}
#endif