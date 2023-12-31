#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "GPUScene.glsl"

layout(push_constant) uniform Params
{
	mat4 M,VP,VPPrev;
	uint ObjectId;
} Parameters;

#ifdef VERTEX_SHADER
	layout (location = 0) out vec3 OutNormal;
	layout (location = 1) out vec2 OutUV;
	layout (location = 2) out uint OutTextureId;
	layout (location = 3) out vec3 OutWP;

	layout (location = 4) out vec4 OutClipspacePos;
	layout (location = 5) out vec4 OutClipspacePosPrev;

	layout (location = 6) out uint OutLightmapId;

	void main()
	{
		GPUSceneMeshCompact Mesh = GPUSceneMeshes.Meshes[Parameters.ObjectId];

		// index is being read from index buffer by gl_VertexIndex
		GPUScene_Vertex Vertex = GPUScene_FetchVertex(Parameters.ObjectId, gl_VertexIndex);

		// TODO: model transformation
		OutWP = Vertex.Position;
		
		vec4 ClipspacePos = Parameters.VP * vec4(Vertex.Position, 1) * vec4(1, -1, 1, 1);
		vec4 ClipspacePosPrev = Parameters.VPPrev * vec4(Vertex.Position, 1) * vec4(1, -1, 1, 1);
		OutClipspacePos = ClipspacePos;
		OutClipspacePosPrev = ClipspacePosPrev;

		gl_Position = ClipspacePos;
		OutNormal = Vertex.Normal;
		OutUV = Vertex.UV;
		OutTextureId = Mesh.TextureId;
		OutLightmapId = Mesh.LightmapId;
	}
#endif

#ifdef PIXEL_SHADER
	layout(location = 0) out vec3 RT0; // diffuse
	layout(location = 1) out vec3 RT1; // normal
	layout(location = 2) out vec3 RT2; // world position
	layout(location = 3) out vec2 RT3; // roughness/metallic
	layout(location = 4) out vec2 RT4; // velocity
	layout(location = 5) out vec3 RT5; // lightmap

	layout (location = 0) in vec3 InNormal;
	layout (location = 1) in vec2 InUV;
	layout (location = 2) in flat uint InTextureId;
	layout (location = 3) in vec3 InWP;
	layout (location = 4) in vec4 InClipspacePos;
	layout (location = 5) in vec4 InClipspacePosPrev;

	layout (location = 6) in flat uint InLightmapId;

	void main()
	{
		vec3 LightmapColor = vec3(0);
		if (InLightmapId != -1)
		{
			LightmapColor = textureLod(Textures[InLightmapId], InUV, 0.0f).rgb;
		}

		vec3 AlbedoColor = vec3(1);
		if (InTextureId != -1)
		{
			AlbedoColor = textureLod(Textures[InTextureId], InUV, 0.0f).rgb;
		}

		RT0 = AlbedoColor;
		RT1 = InNormal;
		RT2 = InWP;
		RT3 = vec2(1, 0);
		RT4 = vec2(InClipspacePos.xy/InClipspacePos.w - InClipspacePosPrev.xy/InClipspacePosPrev.w);
		RT5 = LightmapColor;
	}
#endif
