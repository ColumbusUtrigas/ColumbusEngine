float2 BaryLerp(float2 a, float2 b, float2 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 BaryLerp(float3 a, float3 b, float3 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, BuiltInTriangleIntersectionAttributes attrib)
{
    float3 barycentrics = float3(1.0f - attrib.barycentrics.x - attrib.barycentrics.y, attrib.barycentrics.x, attrib.barycentrics.y);
	
    uint ObjectId = InstanceID();

    GPUSceneMeshCompact Mesh = GPUScene::GPUSceneMeshes[NonUniformResourceIndex(InstanceID())];

    GPUScene::Vertex vert1 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 0));
    GPUScene::Vertex vert2 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 1));
    GPUScene::Vertex vert3 = GPUScene::FetchVertex(NonUniformResourceIndex(ObjectId), NonUniformResourceIndex(PrimitiveIndex() * 3 + 2));

    float2 uv = BaryLerp(vert1.UV, vert2.UV, vert3.UV, barycentrics);
    float3 normal = normalize(BaryLerp(vert1.Normal, vert2.Normal, vert3.Normal, barycentrics));

    int matid = Mesh.MaterialId;

    GPUMaterialSampledData Material = GPUScene::SampleMaterial(matid, uv);

    payload.Colour = Material.Albedo;
    payload.HitDistance = RayTCurrent();
    payload.Normal = normal;
    payload.ObjectId = ObjectId;
    payload.RoughnessMetallic = float2(Material.Roughness, Material.Metallic);
    
#ifdef CLOSEST_HIT_REPORT_FACE
    payload.IsFrontFace = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE;
#endif
}