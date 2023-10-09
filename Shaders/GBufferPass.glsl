#version 460 core
#extension GL_GOOGLE_include_directive : require

#include "GPUScene.glsl"

layout(push_constant) uniform Params
{
	mat4 M,V,P;
	uint ObjectId;
} Parameters;

#ifdef VERTEX_SHADER
	layout (location = 0) out vec3 OutNormal;
	layout (location = 1) out vec2 OutUV;
	layout (location = 2) out uint OutTextureId;
	layout (location = 3) out vec3 OutWP;

	void main()
	{
		uint index = IndexBuffers[Parameters.ObjectId].indices[gl_VertexIndex];
		vec3 vertex = vec3(0);
		vertex.x = VertexBuffers[Parameters.ObjectId].vertices[index * 3 + 0];
		vertex.y = VertexBuffers[Parameters.ObjectId].vertices[index * 3 + 1];
		vertex.z = VertexBuffers[Parameters.ObjectId].vertices[index * 3 + 2];

		vec3 normal = vec3(0);
		normal.x = NormalsBuffers[Parameters.ObjectId].normals[index * 3 + 0];
		normal.y = NormalsBuffers[Parameters.ObjectId].normals[index * 3 + 1];
		normal.z = NormalsBuffers[Parameters.ObjectId].normals[index * 3 + 2];

		OutWP = vertex;
		
		gl_Position = Parameters.P * Parameters.V * vec4(vertex, 1) * vec4(1, -1, 1, 1);
		OutNormal = normal;
		OutUV = UvsBuffers[Parameters.ObjectId].uvs[index];
		OutTextureId = MaterialsBuffers[Parameters.ObjectId].id;
	}
#endif

#ifdef PIXEL_SHADER
	layout(location = 0) out vec3 RT0;
	layout(location = 1) out vec3 RT1;
	layout(location = 2) out vec3 RT2;
	layout(location = 3) out vec2 RT3;

	layout (location = 0) in vec3 InNormal;
	layout (location = 1) in vec2 InUV;
	layout (location = 2) in flat uint InTextureId;
	layout (location = 3) in vec3 InWP;

	void main()
	{
		// RT0 = vec4(InNormal, 1);
		RT0 = textureLod(Textures[InTextureId], InUV, 0.0f).rgb;
		RT1 = InNormal;
		RT2 = InWP;
		RT3 = vec2(1, 0);
	}
#endif
