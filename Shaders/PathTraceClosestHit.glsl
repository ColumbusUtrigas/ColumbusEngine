#ifdef CLOSEST_HIT_SHADER
	#extension GL_EXT_nonuniform_qualifier : require

	#include "GPUScene.glsl"

	vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
		return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
	}

	vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
		return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
	}

	layout(location = 0) rayPayloadInEXT RayPayload payload;

	hitAttributeEXT vec2 HitAttribs;

	void main() {
		vec3 barycentrics = vec3(1.0f - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);

		GPUSceneMeshCompact Mesh = GPUSceneMeshes.Meshes[nonuniformEXT(gl_InstanceCustomIndexEXT)];

		uint index0 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 0];
		uint index1 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 1];
		uint index2 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 2];

		vec2 uv0 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index0];
		vec2 uv1 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index1];
		vec2 uv2 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index2];
		vec2 uv = BaryLerp(uv0, uv1, uv2, barycentrics);

		vec3 normal0 = vec3(NORMALBUF[index0*3+0], NORMALBUF[index0*3+1], NORMALBUF[index0*3+2]);
		vec3 normal1 = vec3(NORMALBUF[index1*3+0], NORMALBUF[index1*3+1], NORMALBUF[index1*3+2]);
		vec3 normal2 = vec3(NORMALBUF[index2*3+0], NORMALBUF[index2*3+1], NORMALBUF[index2*3+2]);
		vec3 normal = BaryLerp(normal0, normal1, normal2, barycentrics);

		int texId = Mesh.TextureId;
		vec3 texel = vec3(1);

		if (texId > -1)
		{
			texel = textureLod(Textures[nonuniformEXT(texId)], uv, 0.0f).rgb;
		}

		payload.colorAndDist = vec4(texel, gl_HitTEXT);
		payload.normalAndObjId = vec4(normal, gl_InstanceCustomIndexEXT);
	}
#endif