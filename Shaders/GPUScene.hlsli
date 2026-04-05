#pragma once

#include "SkyCommon.hlsli"

struct GPUViewCamera
{
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float4x4 ViewProjectionMatrix; // 192

	float4x4 InverseViewMatrix;
	float4x4 InverseProjectionMatrix;
	float4x4 InverseViewProjectionMatrix; // 384
	
	float4 CameraPosition;
	float4 CameraForward;
	float4 CameraUp;
	float4 CameraRight; // 448
	
	float2 Jittering; // 456
	
	int _pad[14]; // 512
};

struct GPUVolumetricFogSettings
{
	float4 AlbedoDensity; // rgb albedo, a density
	float4 Params0; // x height falloff, y height offset, z max distance, w anisotropy
	float4 Params1; // x noise scale, y noise amount, z shadow strength, w unused
	int4 Control; // x enabled, y froxel pixel size, z froxel slices, w integration steps
	int4 Control2; // x max lights
};

struct GPUSceneStruct
{
	// view description
	GPUViewCamera CameraCur;
	GPUViewCamera CameraPrev;
	
    //SkySettings Sky;
	
	int2 RenderSize;
	int2 OutputSize;

	// scene description
    float4 SunDirection; // xyz - direction, w - unused
    float4 SkySHR; // sky SH2 red
    float4 SkySHG; // sky SH2 green
    float4 SkySHB; // sky SH2 blue
	
	uint MeshesCount;
	uint MaterialsCount;
	uint TexturesCount;
	uint LightsCount;
	uint DecalsCount;
	
    uint _pad[3];
	
    SkySettings Sky;
	GPUVolumetricFogSettings VolumetricFog;
};

struct GPULight
{
	float4 Position;
	float4 Direction;
	float4 Color;
	uint   Type;
	float  Range;
	float  SourceRadius;
	float  VolumetricIntensity;
	uint Flags; // 64
    float2 SizeOrSpotAngles; // 76
	
    int _pad[13]; // 128
};

#define GPULIGHT_FLAG_TWOSIDED 1
#define GPULIGHT_FLAG_SHADOW   2

#define GPULIGHT_DIRECTIONAL 0
#define GPULIGHT_POINT 1
#define GPULIGHT_SPOT 2
#define GPULIGHT_RECTANGLE 3
#define GPULIGHT_DISC 4
#define GPULIGHT_LINE 5

struct GPUSceneMeshCompact
{
	float4x4 Transform; // 64
	float4x4 PrevTransform; // 128
    float4x4 NormalMatrix; // 192

	uint64_t VertexBuffer;   // 200
	uint64_t IndexBuffer;    // 208
	uint64_t Uv1Buffer;      // 216
	uint64_t Uv2Buffer;      // 224
	uint64_t NormalsBuffer;  // 232
	uint64_t TangentsBuffer; // 240

	uint VertexCount; // 244
	uint IndexCount;  // 248

	int MaterialId; // 252
	int LightmapId; // 256
};

struct GPUSceneMaterialCompact
{
	float4 AlbedoFactor;   // 16
	float4 EmissiveFactor; // 32

	// textures
	int AlbedoId;
	int NormalId;
	int OrmId;
	int EmissiveId;  // 48

	float Roughness; // 52
	float Metallic;  // 56

	int Flags;
	int _pad; // 64
};

#define GPUMATERIAL_FLAG_NORMAL_RG 1

struct GPUMaterialSampledData
{
    float3 Albedo;
    float3 Emissive;
    float3 Normal; // -1 to 1, tangent space
    float  Occlusion;
    float  Roughness;
    float  Metallic;
};

namespace GPUScene
{
#ifndef GPU_SCENE_NO_BINDINGS
	#define GPUSCENE_TEXTURES_SET 0
	#define GPUSCENE_SCENE_SET 1
	
	[[vk::binding(0, GPUSCENE_TEXTURES_SET)]] Texture2D    Textures[2000];

	[[vk::binding(0, GPUSCENE_SCENE_SET)]] StructuredBuffer<GPUSceneStruct>          GPUSceneScene;
	[[vk::binding(1, GPUSCENE_SCENE_SET)]] StructuredBuffer<GPULight>                GPUSceneLights;
	[[vk::binding(2, GPUSCENE_SCENE_SET)]] StructuredBuffer<GPUSceneMeshCompact>     GPUSceneMeshes;
	[[vk::binding(3, GPUSCENE_SCENE_SET)]] StructuredBuffer<GPUSceneMaterialCompact> GPUSceneMaterials;
    [[vk::binding(4, GPUSCENE_SCENE_SET)]] SamplerState                              GPUSceneSampler;
	
    uint GetLightsCount()
    {
        return GPUSceneScene[0].LightsCount;
    }
	
	struct Vertex
	{
		float3 Position;
		float3 Normal;
		float4 TangentAndSign;
		float2 UV;
		float2 UV2;
		uint   MaterialId;
	};
	
	uint FetchIndex(uint ObjectId, uint Index)
	{
		// 4 is sizeof(uint)
        return vk::RawBufferLoad(GPUSceneMeshes[NonUniformResourceIndex(ObjectId)].IndexBuffer + Index*4);
    }
	
	float3 FetchVertexPositionFromMesh(GPUSceneMeshCompact Mesh, uint Index)
	{
		// 12 is sizeof(float3)
        return vk::RawBufferLoad<float3>(Mesh.VertexBuffer + Index*12);
	}
	
    float3 FetchVertexNormalFromMesh(GPUSceneMeshCompact Mesh, uint Index)
    {
		// 12 is sizeof(float3)
        return vk::RawBufferLoad<float3>(Mesh.NormalsBuffer + Index*12);
    }
	
    float4 FetchVertexTangentFromMesh(GPUSceneMeshCompact Mesh, uint Index)
    {
		// 16 is sizeof(float4)
        return vk::RawBufferLoad<float4>(Mesh.TangentsBuffer + Index*16);
    }

    float2 FetchVertexUVFromMesh(GPUSceneMeshCompact Mesh, uint Index)
    {
		// 8 is sizeof(float2)
        return vk::RawBufferLoad<float2>(Mesh.Uv1Buffer + Index*8);
    }

    float2 FetchVertexUV2FromMesh(GPUSceneMeshCompact Mesh, uint Index)
    {
		// TODO: check if buffer reference is 0 instead
        if (Mesh.LightmapId != -1)
        {
			// 8 is sizeof(float2)
			return vk::RawBufferLoad<float2>(Mesh.Uv2Buffer + Index*8);
        }

        return float2(0,0);
    }

    uint FetchVertexMaterialFromMesh(GPUSceneMeshCompact Mesh, uint Index)
    {
		// TODO: per-vertex materials
        return Mesh.MaterialId;
    }
	
    Vertex FetchVertexByIndex(uint ObjectId, uint Index)
    {
        GPUSceneMeshCompact mesh = GPUSceneMeshes[NonUniformResourceIndex(ObjectId)];

        Vertex vertex;
        vertex.Position       = FetchVertexPositionFromMesh(mesh, Index);
        vertex.Normal         = FetchVertexNormalFromMesh(mesh, Index);
        vertex.TangentAndSign = FetchVertexTangentFromMesh(mesh, Index);
        vertex.UV             = FetchVertexUVFromMesh(mesh, Index);
        vertex.UV2            = FetchVertexUV2FromMesh(mesh, Index);
        vertex.MaterialId     = FetchVertexMaterialFromMesh(mesh, Index);
        return vertex;
    }
	
	// reads index by VertexIndex
    Vertex FetchVertex(uint ObjectId, uint VertexIndex)
    {
        uint index = FetchIndex(ObjectId, VertexIndex);
        return FetchVertexByIndex(ObjectId, index);
    }

    float4 SampleTextureWithDefault(int TextureId, float2 UV, float4 Default)
    {
        if (TextureId != -1)
        {
#if defined(MISS_SHADER) || defined(CLOSEST_HIT_SHADER) || defined(RAYGEN_SHADER)
			return Textures[NonUniformResourceIndex(TextureId)].SampleLevel(GPUSceneSampler, UV, 0);
#else
            return Textures[NonUniformResourceIndex(TextureId)].Sample(GPUSceneSampler, UV);
#endif
			
        }
        else
        {
            return Default;
        }
    }
	
    GPUMaterialSampledData SampleMaterial(uint MaterialId, float2 UV)
    {
        GPUMaterialSampledData Result;

        if (MaterialId != -1)
        {
            GPUSceneMaterialCompact Material = GPUSceneMaterials[NonUniformResourceIndex(MaterialId)];

            Result.Albedo = SampleTextureWithDefault(Material.AlbedoId, UV, float4(1,1,1,1)).rgb * Material.AlbedoFactor.rgb;
            Result.Emissive = SampleTextureWithDefault(Material.EmissiveId, UV, float4(1,1,1,1)).rgb * Material.EmissiveFactor.rgb;

            float4 EncodedNormal = SampleTextureWithDefault(Material.NormalId, UV, float4(0.5, 0.5, 1, 0));
            if ((Material.Flags & GPUMATERIAL_FLAG_NORMAL_RG) != 0)
            {
                float2 NormalXY = EncodedNormal.rg * 2.0f - 1.0f;
                NormalXY.y *= -1.0f; // OpenGL to DX convention
                float NormalZ = sqrt(saturate(1.0f - dot(NormalXY, NormalXY)));
                Result.Normal = normalize(float3(NormalXY, NormalZ));
            }
            else
            {
                Result.Normal = normalize(EncodedNormal.rgb * 2.0f - 1.0f);
                Result.Normal.y *= -1.0f; // OpenGL to DX convention
            }

            float3 ORM = SampleTextureWithDefault(Material.OrmId, UV, float4(1, Material.Roughness, Material.Metallic, 1)).rgb;

            Result.Occlusion = ORM.r;
            Result.Roughness = ORM.g;
            Result.Metallic = ORM.b;
        }
        else
        {
            Result.Albedo = float3(1,1,1);
            Result.Emissive = float3(0,0,0);
            Result.Normal = float3(0, 0, 1);
            Result.Occlusion = 1;
            Result.Roughness = 1;
            Result.Metallic = 0;
        }

        return Result;
    }
#endif // GPU_SCENE_NO_BINDINGS

} // namespace GPUScene
