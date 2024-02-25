#include "World.h"

#include <algorithm>
#include <Lib/tinygltf/tiny_gltf.h>

IMPLEMENT_MEMORY_PROFILING_COUNTER("Textures", "SceneMemory", MemoryCounter_SceneTextures);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Meshes", "SceneMemory", MemoryCounter_SceneMeshes);
IMPLEMENT_MEMORY_PROFILING_COUNTER("BLAS", "SceneMemory", MemoryCounter_SceneBLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("TLAS", "SceneMemory", MemoryCounter_SceneTLAS);

namespace Columbus
{

	Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data);
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, CPUScene& cpuScene, std::vector<Box>& Boundings);

	void EngineWorld::LoadLevelGLTF(const char* Path)
	{
		SceneGPU = LoadScene(Device, MainView.CameraCur, Path, SceneCPU, MeshBoundingBoxes);
	}

	WorldIntersectionResult EngineWorld::CastRayClosestHit(const Geometry::Ray& Ray)
	{
		// TODO: remove allocations, use some temporary frame allocator
		std::vector<WorldIntersectionResult> HitPoints;

		// TODO: BVH/octree search
		for (int i = 0; i < MeshBoundingBoxes.size(); i++)
		{
			Box& Bounding = MeshBoundingBoxes[i];

			if (Bounding.Intersects(Ray.Origin, Ray.Direction))
			{
				CPUSceneMesh& Mesh = SceneCPU.Meshes[i];
				for (int v = 0; v < Mesh.Indices.size(); v += 3)
				{
					// extract triangle. TODO: mesh processing functions
					u32 i1 = Mesh.Indices[v + 0];
					u32 i2 = Mesh.Indices[v + 1];
					u32 i3 = Mesh.Indices[v + 2];

					Geometry::Triangle Tri{
						Mesh.Vertices[i1],
						Mesh.Vertices[i2],
						Mesh.Vertices[i3],
					};

					Geometry::HitPoint IntersectionPoint = Geometry::RayTriangleIntersection(Ray, Tri);
					if (IntersectionPoint.IsHit)
					{
						WorldIntersectionResult Intersection{
							.HasIntersection = true,
							.MeshId = i,
							.TriangleId = v / 3,
							.IntersectionPoint = IntersectionPoint.Point,
							.Triangle = Tri,
						};

						HitPoints.push_back(Intersection);
					}
				}
			}
		}

		std::sort(HitPoints.begin(), HitPoints.end(), [Ray](const WorldIntersectionResult& L, const WorldIntersectionResult& R)
		{
			return L.IntersectionPoint.Length(Ray.Origin) < R.IntersectionPoint.Length(Ray.Origin);
		});

		if (HitPoints.size() > 0)
		{
			return HitPoints[0];
		}

		return WorldIntersectionResult::Invalid();
	}

	WorldIntersectionResult EngineWorld::CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates)
	{
		Vector2 NDC = NormalisedScreenCoordinates * 2 - 1;
		NDC.Y = -NDC.Y;

		Geometry::Ray CameraRay{
			.Origin = MainView.CameraCur.Pos,
			.Direction = MainView.CameraCur.CalcRayByNdc(NDC)
		};

		return CastRayClosestHit(CameraRay);
	}

	void EngineWorld::BeginFrame()
	{
		ResetProfiling();

		MainView.CameraPrev = MainView.CameraCur;
		MainView.DebugRender.Clear();
	}

	void EngineWorld::EndFrame()
	{
	}

	void EngineWorld::FreeResources()
	{
		SceneCPU = CPUScene();
		SceneGPU = nullptr;
		MeshBoundingBoxes.clear();
	}

	// TODO: move to appropriate place
	Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data)
	{
		BufferDesc desc;
		desc.BindFlags = BufferType::UAV;
		desc.Size = size;
		desc.UsedInAccelerationStructure = usedInAS;
		Buffer* result = device->CreateBuffer(desc, data);
		AddProfilingMemory(MemoryCounter_SceneMeshes, result->GetSize());
		return result;
	}

	// TODO: move to appropriate place
	// TODO: separate CPUScene load and GPUScene load?
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, CPUScene& cpuScene, std::vector<Box>& Boundings)
	{
		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err, warn;

		Timer GltfTimer;
		if (!loader.LoadASCIIFromFile(&model, &err, &warn, Filename))
		{
			Log::Fatal("Couldn't load scene, %s", Filename.c_str());
		}
		Log::Message("GLTF loaded, time: %0.2f s", GltfTimer.Elapsed());

		SPtr<GPUScene> Scene = SPtr<GPUScene>(GPUScene::CreateGPUScene(Device), [Device](GPUScene* Scene)
			{
				for (auto& Texture : Scene->Textures)
				{
					Device->DestroyTexture(Texture);
				}

				for (auto& Mesh : Scene->Meshes)
				{
					Device->DestroyBuffer(Mesh.Vertices);
					Device->DestroyBuffer(Mesh.Indices);
					Device->DestroyBuffer(Mesh.UV1);
					if (Mesh.UV2)
						Device->DestroyBuffer(Mesh.UV2);
					Device->DestroyBuffer(Mesh.Normals);
					Device->DestroyAccelerationStructure(Mesh.BLAS);
				}

				for (auto& Decal : Scene->Decals)
				{
					Device->DestroyTexture(Decal.Texture);
				}

				Device->DestroyAccelerationStructure(Scene->TLAS);

				GPUScene::DestroyGPUScene(Scene, Device);
			});

		auto CreateTexture = [Scene, Device, &model](int textureId, const char* name, TextureFormat format) -> int
		{
			auto& image = model.images[textureId];

			Image img;
			img.Format = format;
			img.FromMemory(image.image.data(), image.image.size(), image.width, image.height);

			auto tex = Device->CreateTexture(img);
			Device->SetDebugName(tex, name);
			AddProfilingMemory(MemoryCounter_SceneTextures, tex->GetSize());

			int id = (int)Scene->Textures.size();
			Scene->Textures.push_back(tex);

			return id;
		};

		Timer MeshTimer;
		// TODO
		for (auto& mesh : model.meshes)
		{
			for (auto& primitive : mesh.primitives)
			{
				Buffer* indexBuffer = nullptr;
				Buffer* vertexBuffer = nullptr;
				Buffer* uvBuffer = nullptr;
				Buffer* normalBuffer = nullptr;
				Buffer* materialBuffer = nullptr;

				Vector3 MinVertex(FLT_MAX);
				Vector3 MaxVertex(-FLT_MAX);

				cpuScene.Meshes.push_back(CPUSceneMesh{});
				CPUSceneMesh& cpuMesh = cpuScene.Meshes.back();

				int indicesCount = 0;
				int verticesCount = 0;

				{
					auto accessor = model.accessors[primitive.indices];
					auto view = model.bufferViews[accessor.bufferView];
					auto buffer = model.buffers[view.buffer];
					auto offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					std::vector<uint32_t> indices(accessor.count);
					indicesCount = accessor.count;

					switch (accessor.componentType)
					{
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						for (int i = 0; i < accessor.count; i++)
						{
							indices[i] = static_cast<const uint16_t*>(data)[i];
						}
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
						for (int i = 0; i < accessor.count; i++)
						{
							indices[i] = static_cast<const uint32_t*>(data)[i];
						}
						break;
					default: COLUMBUS_ASSERT(false);
					}

					cpuMesh.Indices = indices;

					indexBuffer = CreateMeshBuffer(Device, indices.size() * sizeof(uint32_t), true, indices.data());
					Device->SetDebugName(indexBuffer, (mesh.name + " (Indices)").c_str());
				}

				{
					auto accessor = model.accessors[primitive.attributes["POSITION"]];
					auto view = model.bufferViews[accessor.bufferView];
					auto buffer = model.buffers[view.buffer];
					auto offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					cpuMesh.Vertices = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data)+verticesCount);

					for (const Vector3& Vertex : cpuMesh.Vertices)
					{
						MinVertex.X = Math::Min(MinVertex.X, Vertex.X);
						MinVertex.Y = Math::Min(MinVertex.Y, Vertex.Y);
						MinVertex.Z = Math::Min(MinVertex.Z, Vertex.Z);

						MaxVertex.X = Math::Max(MaxVertex.X, Vertex.X);
						MaxVertex.Y = Math::Max(MaxVertex.Y, Vertex.Y);
						MaxVertex.Z = Math::Max(MaxVertex.Z, Vertex.Z);
					}

					Boundings.push_back(Box(MinVertex, MaxVertex));

					vertexBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
					Device->SetDebugName(vertexBuffer, (mesh.name + " (Vertices)").c_str());
				}

				{
					auto accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
					auto view = model.bufferViews[accessor.bufferView];
					auto buffer = model.buffers[view.buffer];
					auto offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					cpuMesh.UV1 = std::vector<Vector2>((Vector2*)(data), (Vector2*)(data)+verticesCount);

					uvBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector2), true, data);
					Device->SetDebugName(uvBuffer, (mesh.name + " (UVs)").c_str());
				}

				{
					auto accessor = model.accessors[primitive.attributes["NORMAL"]];
					auto view = model.bufferViews[accessor.bufferView];
					auto buffer = model.buffers[view.buffer];
					auto offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					cpuMesh.Normals = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data)+verticesCount);

					normalBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
					Device->SetDebugName(normalBuffer, (mesh.name + " (Normals)").c_str());
				}

				Columbus::AccelerationStructureDesc blasDesc;
				blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
				blasDesc.Vertices = vertexBuffer;
				blasDesc.Indices = indexBuffer;
				blasDesc.VerticesCount = verticesCount;
				blasDesc.IndicesCount = indicesCount;
				auto BLAS = Device->CreateAccelerationStructure(blasDesc);

				if (Device->SupportsRayTracing())
				{
					Device->SetDebugName(BLAS, mesh.name.c_str());
					AddProfilingMemory(MemoryCounter_SceneBLAS, BLAS->GetSize());
				}

				int matid = -1;

				if (primitive.material > -1)
				{
					auto mat = model.materials[primitive.material];

					int modelAlbedoId = mat.pbrMetallicRoughness.baseColorTexture.index;
					int albedoId = CreateTexture(model.textures[modelAlbedoId].source, model.textures[modelAlbedoId].name.c_str(), TextureFormat::RGBA8SRGB);

					matid = albedoId;
				}

				GPUSceneMesh Mesh;
				Mesh.BLAS = BLAS;
				Mesh.Transform = Matrix(1);
				Mesh.Vertices = vertexBuffer;
				Mesh.Indices = indexBuffer;
				Mesh.UV1 = uvBuffer;
				Mesh.Normals = normalBuffer;
				Mesh.VertexCount = verticesCount;
				Mesh.IndicesCount = indicesCount;
				Mesh.TextureId = matid;

				Scene->Meshes.push_back(Mesh);
			}
		}
		Log::Message("Meshes loaded, time: %0.2f s", MeshTimer.Elapsed());

		// TLAS and BLASes should be packed into GPU scene
		AccelerationStructureDesc TlasDesc;
		TlasDesc.Type = AccelerationStructureType::TLAS;
		TlasDesc.Instances = {};
		for (auto& mesh : Scene->Meshes)
		{
			TlasDesc.Instances.push_back({ Matrix(), mesh.BLAS });
		}

		Scene->TLAS = Device->CreateAccelerationStructure(TlasDesc);
		Scene->MainCamera = GPUCamera(DefaultCamera);

		if (Device->SupportsRayTracing())
		{
			AddProfilingMemory(MemoryCounter_SceneTLAS, Scene->TLAS->GetSize());
		}

		Scene->Lights.push_back(GPULight{ {}, {1,1,1,0}, {1,1,1,0}, LightType::Directional, 0, 0.1f });
		Scene->Lights.push_back(GPULight{ {0,200,0,0}, {}, {50,0,0,0}, LightType::Point, 500, 0 });

		{
			Image DecalImage;
			if (!DecalImage.Load("./Data/Textures/Detail.dds"))
			{
				Log::Error("Couldn't load decal image");
			}

			Texture2* DecalTexture = Device->CreateTexture(DecalImage);

			Matrix Decal;
			Decal.Scale({ 100 });
			Scene->Decals.push_back(GPUDecal{ Decal, Decal.GetInverted(), DecalTexture });
		}

		return Scene;
	}
}
