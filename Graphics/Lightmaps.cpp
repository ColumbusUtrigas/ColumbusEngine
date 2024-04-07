#include "Lightmaps.h"
#include "World.h"

// Third party
#include "Lib/xatlas/xatlas.h"

namespace Columbus
{

	LightmapMesh::~LightmapMesh()
	{
		if (Device)
		{
			if (VertexBuffer != nullptr)
			{
				Device->DestroyBuffer(VertexBuffer);
			}

			if (IndexBuffer)
			{
				Device->DestroyBuffer(IndexBuffer);
			}
		}
	}

	void LightmapSystem::Clear()
	{
		Atlas.Width = 0;
		Atlas.Height = 0;
		BakingRequested = false;
		Meshes.clear();
	}

	void GenerateAndPackLightmaps(EngineWorld& World)
	{
		// TODO: after refactoring of the World, UV2 storage is broken
#if 0
		LightmapSystem& System = World.Lightmaps;

		System.Clear();

		xatlas::Atlas* atlas = xatlas::Create();

		for (const GameObject& GO : World.GameObjects)
		{
			for (const MeshPrimitive& Primitive : World.Meshes[GO.MeshId].Primitives)
			{
				const CPUMeshResource& Mesh = Primitive.CPU;
				//const Matrix Transform = GO.Trans.GetMatrix();

				xatlas::MeshDecl xmesh
				{
					.vertexPositionData = Mesh.Vertices.data(),
					.vertexNormalData = Mesh.Normals.data(),
					.vertexUvData = Mesh.UV1.data(),
					.indexData = Mesh.Indices.data(),

					.vertexCount = (u32)Mesh.Vertices.size(),
					.vertexPositionStride = sizeof(Vector3),
					.vertexNormalStride = sizeof(Vector3),
					.vertexUvStride = sizeof(Vector2),
					.indexCount = (u32)Mesh.Indices.size(),

					.indexFormat = xatlas::IndexFormat::UInt32,
				};

				xatlas::AddMesh(atlas, xmesh);
			}
		}

		xatlas::ChartOptions chartOptions;
		chartOptions.useInputMeshUvs = true;
		xatlas::PackOptions packOptions;
		packOptions.createImage = true;
		xatlas::Generate(atlas, chartOptions, packOptions);

		System.Atlas.Width = atlas->width;
		System.Atlas.Height = atlas->height;

		for (u32 i = 0; i < atlas->meshCount; i++)
		{
			xatlas::Mesh& mesh = atlas->meshes[i];

			Scene.Meshes[i].UV2.resize(Scene.Meshes[i].UV1.size());

			System.Meshes.push_back(LightmapMesh{});
			System.Meshes[i].Vertices.resize(mesh.vertexCount);

			for (u32 v = 0; v < mesh.vertexCount; v++)
			{
				System.Meshes[i].Vertices[v] = LightmapMeshVertex {
					.Position = Scene.Meshes[i].Vertices[mesh.vertexArray[v].xref],
					.Normal = Scene.Meshes[i].Normals[mesh.vertexArray[v].xref],
					.UV2 = Vector2(mesh.vertexArray[v].uv[0] / (float)atlas->width, mesh.vertexArray[v].uv[1] / (float)atlas->height),
				};

				if (mesh.vertexArray[v].chartIndex != -1)
				{
					Scene.Meshes[i].UV2[mesh.vertexArray[v].xref] = Vector2(mesh.vertexArray[v].uv[0] / (float)atlas->width, mesh.vertexArray[v].uv[1] / (float)atlas->height);
				}
			}

			System.Meshes[i].Indices.resize(mesh.indexCount);
			for (u32 v = 0; v < mesh.indexCount; v++)
			{
				System.Meshes[i].Indices[v] = mesh.indexArray[v];
			}
		}

		xatlas::Destroy(atlas);
#endif
	}

	void UploadLightmapMeshesToGPU(EngineWorld& World)
	{
		// TODO: after refactoring of the World, UV2 storage is broken
#if 0
		LightmapSystem& System = World.Lightmaps;

		for (u32 i = 0; i < System.Meshes.size(); i++)
		{
			LightmapMesh& Mesh = System.Meshes[i];
			Mesh.Device = World.Device;

			BufferDesc VerticesDesc;
			VerticesDesc.BindFlags = BufferType::UAV;
			VerticesDesc.Size = Mesh.Vertices.size() * sizeof(Mesh.Vertices[0]);

			BufferDesc IndicesDesc;
			IndicesDesc.BindFlags = BufferType::UAV;
			IndicesDesc.Size = Mesh.Indices.size() * sizeof(Mesh.Indices[0]);

			Mesh.VertexBuffer = World.Device->CreateBuffer(VerticesDesc, Mesh.Vertices.data());
			Mesh.IndexBuffer = World.Device->CreateBuffer(IndicesDesc, Mesh.Indices.data());

			char VertexName[256]{0};
			char IndexName[256]{0};

			snprintf(VertexName, 256, "LightmapMesh Vertices %i", i);
			snprintf(IndexName, 256, "LightmapMesh Indices %i", i);

			World.Device->SetDebugName(Mesh.VertexBuffer, VertexName);
			World.Device->SetDebugName(Mesh.IndexBuffer, IndexName);
		}

		// create lightmap
		{
			// TODO: lightmap texture creation/management
			TextureDesc2 LightmapTextureDesc {
				.Usage = TextureUsage::StorageSampled,
				.Width = System.Atlas.Width, .Height = System.Atlas.Height,
				.Format = TextureFormat::RGBA16F,
			};

			System.Atlas.Lightmap = World.Device->CreateTexture(LightmapTextureDesc);
			World.Device->SetDebugName(System.Atlas.Lightmap, "Lightmap");
		}

		// register lightmap
		int LightmapTexId = (int)GpuScene->Textures.size();
		GpuScene->Textures.push_back(System.Atlas.Lightmap);

		// assign lightmap to meshes
		for (u32 i = 0; i < GpuScene->Meshes.size(); i++)
		{
			GPUSceneMesh& GpuMesh = GpuScene->Meshes[i];
			GpuMesh.LightmapId = LightmapTexId;
		}

		// TODO: move to CPU/GPU scene management
		for (u32 i = 0; i < GpuScene->Meshes.size(); i++)
		{
			CPUSceneMesh& CpuMesh = CpuScene.Meshes[i];
			GPUSceneMesh& GpuMesh = GpuScene->Meshes[i];

			BufferDesc UV2Desc;
			UV2Desc.BindFlags = BufferType::UAV;
			UV2Desc.Size = CpuMesh.UV2.size() * sizeof(CpuMesh.UV2[0]);

			GpuMesh.MeshResource->UV2 = Device->CreateBuffer(UV2Desc, CpuMesh.UV2.data());

			char Uv2Name[256]{0};
			snprintf(Uv2Name, 256, "Mesh UV2 %i", i);
			Device->SetDebugName(GpuMesh.MeshResource->UV2, Uv2Name);
		}
#endif
	}

}
