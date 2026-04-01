float2 BaryLerp(float2 a, float2 b, float2 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float3 BaryLerp(float3 a, float3 b, float3 c, float3 barycentrics)
{
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

float4 BaryLerp(float4 a, float4 b, float4 c, float3 barycentrics)
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
    float3 position1 = mul(float4(vert1.Position, 1), Mesh.Transform).xyz;
    float3 position2 = mul(float4(vert2.Position, 1), Mesh.Transform).xyz;
    float3 position3 = mul(float4(vert3.Position, 1), Mesh.Transform).xyz;

    float3 geometricNormal = normalize(cross(position2 - position1, position3 - position1));

    float3x3 normalMatrix = (float3x3)0;
    normalMatrix[0] = Mesh.NormalMatrix[0].xyz;
    normalMatrix[1] = Mesh.NormalMatrix[1].xyz;
    normalMatrix[2] = Mesh.NormalMatrix[2].xyz;

    float3 normal = normalize(mul(BaryLerp(vert1.Normal, vert2.Normal, vert3.Normal, barycentrics), normalMatrix));
    float4 tangentAndSign = BaryLerp(vert1.TangentAndSign, vert2.TangentAndSign, vert3.TangentAndSign, barycentrics);
    float3 tangent = normalize(mul(tangentAndSign.xyz, normalMatrix));
    tangent = normalize(tangent - normal * dot(normal, tangent));
    float tangentSign = tangentAndSign.w >= 0.0 ? 1.0 : -1.0;
    float3 bitangent = normalize(cross(normal, tangent)) * tangentSign;

    int matid = Mesh.MaterialId;

    GPUMaterialSampledData Material = GPUScene::SampleMaterial(matid, uv);
    float3x3 TBN = float3x3(tangent, bitangent, normal);
    float3 shadingNormal = normalize(mul(Material.Normal, TBN));

    if (dot(geometricNormal, -WorldRayDirection()) < 0.0)
    {
        geometricNormal = -geometricNormal;
        normal = -normal;
        shadingNormal = -shadingNormal;
    }

    if (dot(shadingNormal, geometricNormal) < 0.0)
    {
        shadingNormal = geometricNormal;
    }

    payload.Colour = Material.Albedo;
    payload.HitDistance = RayTCurrent();
    payload.Normal = shadingNormal;
#ifdef PAYLOAD_HAS_GEOMETRIC_NORMAL
    payload.GeometricNormal = geometricNormal;
#endif
    payload.ObjectId = ObjectId;
    payload.RoughnessMetallic = float2(Material.Roughness, Material.Metallic);
    
#ifdef CLOSEST_HIT_REPORT_FACE
    payload.IsFrontFace = HitKind() == HIT_KIND_TRIANGLE_FRONT_FACE;
#endif
}
