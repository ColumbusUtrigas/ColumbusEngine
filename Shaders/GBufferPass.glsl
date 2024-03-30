#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "GPUScene.glsl"

layout(push_constant) uniform Params
{
	mat4 M,VP,VPPrev; // TODO: remove that from here
	uint ObjectId;
} Parameters;

#ifdef VERTEX_SHADER
	layout (location = 0) out vec3 OutNormal;
	layout (location = 1) out vec2 OutUV;
	layout (location = 2) out vec2 OutUV2;
	layout (location = 3) out uint OutMaterialId;
	layout (location = 4) out vec3 OutWP;

	layout (location = 5) out vec4 OutClipspacePos;
	layout (location = 6) out vec4 OutClipspacePosPrev;

	layout (location = 7) out uint OutLightmapId;

	layout (location = 8) out mat3 TBN;

	void main()
	{
		GPUSceneMeshCompact Mesh = GPUSceneMeshes.Meshes[Parameters.ObjectId];

		// index is being read from index buffer by gl_VertexIndex
		GPUScene_Vertex Vertex = GPUScene_FetchVertex(Parameters.ObjectId, gl_VertexIndex);

		// TODO: precompute and store this matrix in the buffer
		mat3 NormalMatrix = transpose(inverse(transpose(mat3(Mesh.Transform))));
		vec3 Normal = normalize(NormalMatrix * Vertex.Normal);
		vec3 Tangent = normalize(NormalMatrix * Vertex.TangentAndSign.xyz);
		vec3 Bitangent = cross(Normal, Tangent) * Vertex.TangentAndSign.w;

		TBN = mat3(Tangent, Bitangent, Normal);

		// TODO: fucked up row/column major?
		vec4 TransformedPos = vec4(Vertex.Position, 1) * Mesh.Transform;
		OutWP = TransformedPos.xyz;
		
		vec4 ClipspacePos = TransformedPos * Parameters.VP * vec4(1, -1, 1, 1);
		vec4 ClipspacePosPrev = TransformedPos * Parameters.VPPrev * vec4(1, -1, 1, 1);
		OutClipspacePos = ClipspacePos;
		OutClipspacePosPrev = ClipspacePosPrev;

		gl_Position = ClipspacePos;
		OutNormal = Normal;
		OutUV = Vertex.UV;
		OutUV2 = Vertex.UV2;
		OutMaterialId = Mesh.MaterialId;
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

	layout (location = 0) in vec3 InNormal; // TODO: don't need this anymore?
	layout (location = 1) in vec2 InUV;
	layout (location = 2) in vec2 InUV2;
	layout (location = 3) in flat uint InMaterialId;
	layout (location = 4) in vec3 InWP;
	layout (location = 5) in vec4 InClipspacePos;
	layout (location = 6) in vec4 InClipspacePosPrev;

	layout (location = 7) in flat uint InLightmapId;

	layout (location = 8) in mat3 TBN;

	void main()
	{
		vec3 LightmapColor = vec3(0);
		if (InLightmapId != -1)
		{
			// TODO: bicubic lightmap sampling
			LightmapColor = textureLod(Textures[InLightmapId], InUV2, 0.0f).rgb;
		}

		GPUMaterialSampledData Material = GPUScene_SampleMaterial(InMaterialId, InUV);

		RT0 = Material.Albedo;
		// RT1 = normalize(TBN * Material.Normal);
		RT1 = normalize(InNormal);
		RT2 = InWP; // TODO: remove? reconstruct from screenpos and linear depth
		RT3 = vec2(Material.Roughness, Material.Metallic);
		RT4 = vec2(InClipspacePos.xy/InClipspacePos.w - InClipspacePosPrev.xy/InClipspacePosPrev.w);
		RT5 = LightmapColor;
	}
#endif
