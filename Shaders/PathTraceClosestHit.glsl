#ifdef CLOSEST_HIT_SHADER
	// #extension GL_EXT_nonuniform_qualifier : require

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

		GPUScene_Vertex vert1 = GPUScene_FetchVertex(nonuniformEXT(gl_InstanceCustomIndexEXT), nonuniformEXT(gl_PrimitiveID * 3 + 0));
		GPUScene_Vertex vert2 = GPUScene_FetchVertex(nonuniformEXT(gl_InstanceCustomIndexEXT), nonuniformEXT(gl_PrimitiveID * 3 + 1));
		GPUScene_Vertex vert3 = GPUScene_FetchVertex(nonuniformEXT(gl_InstanceCustomIndexEXT), nonuniformEXT(gl_PrimitiveID * 3 + 2));

		vec2 uv = BaryLerp(vert1.UV, vert2.UV, vert3.UV, barycentrics);
		vec3 normal = normalize(BaryLerp(vert1.Normal, vert2.Normal, vert3.Normal, barycentrics));

		int matid = Mesh.MaterialId;

		GPUMaterialSampledData Material = GPUScene_SampleMaterial(matid, uv);

		payload.colorAndDist = vec4(Material.Albedo, gl_HitTEXT);
		payload.normalAndObjId = vec4(normal, gl_InstanceCustomIndexEXT);
		payload.RoughnessMetallic = vec2(Material.Roughness, Material.Metallic);
	}
#endif