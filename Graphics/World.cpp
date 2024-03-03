#include "World.h"

#include <Math/Quaternion.h>

#include <algorithm>
#include <Lib/tinygltf/tiny_gltf.h>

IMPLEMENT_MEMORY_PROFILING_COUNTER("Textures", "SceneMemory", MemoryCounter_SceneTextures);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Meshes", "SceneMemory", MemoryCounter_SceneMeshes);
IMPLEMENT_MEMORY_PROFILING_COUNTER("BLAS", "SceneMemory", MemoryCounter_SceneBLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("TLAS", "SceneMemory", MemoryCounter_SceneTLAS);

IMPLEMENT_CPU_PROFILING_COUNTER("Transforms Update", "SceneCPU", CpuCounter_SceneTransformUpdate);

namespace Columbus
{

	Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data);
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, CPUScene& cpuScene, std::vector<Box>& Boundings, std::vector<GameObject>& GameObjects);

	void EngineWorld::LoadLevelGLTF(const char* Path)
	{
		SceneGPU = LoadScene(Device, MainView.CameraCur, Path, SceneCPU, MeshBoundingBoxes, GameObjects);
	}

	void EngineWorld::ReparentGameObject(GameObjectId Object, GameObjectId NewParent)
	{
		GameObjectId OldParent = GameObjects[Object].ParentId;
		if (OldParent != -1)
		{
			// TODO: proper data structures
			auto Iter = std::find(GameObjects[OldParent].Children.begin(), GameObjects[OldParent].Children.end(), Object);
			if (Iter != GameObjects[OldParent].Children.end())
				GameObjects[OldParent].Children.erase(Iter);
		}

		GameObjects[Object].ParentId = NewParent;
		GameObjects[NewParent].Children.push_back(Object);
	}

	WorldIntersectionResult EngineWorld::CastRayClosestHit(const Geometry::Ray& Ray)
	{
		// TODO: remove allocations, use some temporary frame allocator
		std::vector<WorldIntersectionResult> HitPoints;

		// TODO: BVH/octree search
		// TODO: apply transformations
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

	void EngineWorld::UpdateTransforms()
	{
		PROFILE_CPU(CpuCounter_SceneTransformUpdate);

		// TODO: optimise

		for (int i = 0; i < (int)GameObjects.size(); i++)
		{
			GameObject& Object = GameObjects[i];
			Transform& Trans = Object.Trans;

			Trans.Update();

			Matrix GlobalTransform = Trans.GetMatrix();

			int ParentId = Object.ParentId;

			while (ParentId != -1)
			{
				GameObject& ParentObj = GameObjects[ParentId];
				GlobalTransform = ParentObj.Trans.GetMatrix() * GlobalTransform;
				ParentId = ParentObj.ParentId;
			}

			// TODO: remove mesh duplication
			if (Object.MeshId != -1)
			{
				SceneGPU->Meshes[Object.MeshId].Transform = GlobalTransform;
			}
		}
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

	enum GltfLoadTimerType
	{
		GltfLoadBuffer,
		GltfLoadMaterial,
		GltfLoadMax,
	};

	float GltfLoadTimes[GltfLoadMax]{ 0 };

	template <GltfLoadTimerType Type>
	struct GltfLoadTimer
	{
		Timer T;
		~GltfLoadTimer()
		{
			GltfLoadTimes[Type] += T.Elapsed();
		}
	};

	// TODO: separate CPUScene load and GPUScene load?
	// TODO: refactor this completely
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, CPUScene& cpuScene, std::vector<Box>& Boundings, std::vector<GameObject>& GameObjects)
	{
		const tinygltf::LoadImageDataFunction LoadImageFn = [](tinygltf::Image* Img, const int ImgId, std::string* err, std::string* warn,
			int req_width, int req_height, const unsigned char* bytes, int size, void* user_data) -> bool
		{
			u32 W, H, Mips;
			TextureFormat Format;
			ImageType Type;

			DataStream Stream = DataStream::CreateFromMemory((u8*)bytes, size);

			u8* Data = nullptr;
			if (!ImageUtils::ImageLoadFromStream(Stream, W, H, Mips, Format, Type, Data))
			{
				delete[] Data;
				return false;
			}

			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);

			u64 Size = size_t(W * H) * size_t(FormatInfo.BitsPerPixel / 8);

			Img->width = W;
			Img->height = H;
			Img->component = FormatInfo.NumChannels;
			Img->bits = FormatInfo.BitsPerPixel;
			Img->pixel_type = (int)Format; // our small hack here
			Img->image.resize(Size);
			memcpy(Img->image.data(), Data, Size);
			delete[] Data;
			return true;
		};

		tinygltf::Model model;
		tinygltf::TinyGLTF loader;
		std::string err, warn;

		loader.SetImageLoader(LoadImageFn, nullptr);

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

		std::unordered_map<int, int> LoadedTextures;

		auto CreateTexture = [Scene, Device, &model, &LoadedTextures](int textureId, const char* name) -> int
		{
			if (LoadedTextures.contains(textureId))
			{
				return LoadedTextures[textureId];
			}

			GltfLoadTimer<GltfLoadMaterial> T;

			auto& image = model.images[textureId];
			TextureFormat format = (TextureFormat)image.pixel_type;

			// TODO: proper mechanism
			if (format == TextureFormat::RGBA8)
				format = TextureFormat::RGBA8SRGB;

			// TODO: WTF
			// TODO: GPUs don't really support RGB8, make a layer to convert it to RGBA8 instead
			if (format == TextureFormat::RGB8)
				format = TextureFormat::RGB8SRGB;

			Image img;
			img.Format = format;
			img.Width = image.width;
			img.Height = image.height;
			img.Size = image.image.size();
			img.Data = image.image.data();
			img.MipMaps = 1;

			Texture2* tex = Device->CreateTexture(img);
			Device->SetDebugName(tex, name);
			AddProfilingMemory(MemoryCounter_SceneTextures, tex->GetSize());

			img.Data = nullptr; // so it doesn't get deleted here

			int id = (int)Scene->Textures.size();
			Scene->Textures.push_back(tex);
			LoadedTextures[textureId] = id;

			return id;
		};

		Timer MeshTimer;
		// TODO: proper data loading, materials, scene graph

		{
			// get nodes and transforms
			for (int i = 0; i < model.nodes.size(); i++)
			{
				tinygltf::Node& Node = model.nodes[i];
				GameObject Object;

				Matrix LocalTransform(1);
				if (Node.matrix.size() > 0)
				{
					// TODO: support it
					assert(false);
					memcpy(LocalTransform.M, Node.matrix.data(), sizeof(float) * 16);
					LocalTransform.Transpose();
				}
				else
				{
					if (Node.scale.size() > 0)
					{
						LocalTransform.Scale(Vector3(Node.scale[0], Node.scale[1], Node.scale[2]));
						Object.Trans.Scale = Vector3(Node.scale[0], Node.scale[1], Node.scale[2]);
					}

					if (Node.rotation.size() > 0)
					{
						Quaternion Quat(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);
						LocalTransform *= Quat.ToMatrix();

						Object.Trans.Rotation = Quat;
					}

					if (Node.translation.size() > 0)
					{
						LocalTransform.Translate(Vector3(Node.translation[0], Node.translation[1], Node.translation[2]));
						Object.Trans.Position = Vector3(Node.translation[0], Node.translation[1], Node.translation[2]);
					}
				}

				Object.Id = i;
				Object.Children = Node.children; // vector<int>
				Object.MeshId = Node.mesh;
				Object.Name = Node.name;

				GameObjects.push_back(Object);
			}

			// compute parents
			for (int i = 0; i < GameObjects.size(); i++)
			{
				GameObject& Object = GameObjects[i];
				for (int Child : Object.Children)
				{
					GameObjects[Child].ParentId = i;
				}
			}
		}

		//for (tinygltf::Node& Node : model.nodes)
		for (int NodeId = 0; NodeId < model.nodes.size(); NodeId++)
		{
			tinygltf::Node& Node = model.nodes[NodeId];

			if (Node.mesh == -1) continue;

			tinygltf::Mesh& mesh = model.meshes[Node.mesh];

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
					const auto& accessor = model.accessors[primitive.indices];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
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

					{
						GltfLoadTimer<GltfLoadBuffer> T;
						indexBuffer = CreateMeshBuffer(Device, indices.size() * sizeof(uint32_t), true, indices.data());
						Device->SetDebugName(indexBuffer, (mesh.name + " (Indices)").c_str());
					}
				}

				{
					const auto& accessor = model.accessors[primitive.attributes["POSITION"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
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

					{
						GltfLoadTimer<GltfLoadBuffer> T;
						vertexBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
						Device->SetDebugName(vertexBuffer, (mesh.name + " (Vertices)").c_str());
					}
				}

				{
					const auto& accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					cpuMesh.UV1 = std::vector<Vector2>((Vector2*)(data), (Vector2*)(data)+verticesCount);

					{
						GltfLoadTimer<GltfLoadBuffer> T;
						uvBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector2), true, data);
						Device->SetDebugName(uvBuffer, (mesh.name + " (UVs)").c_str());
					}
				}

				{
					const auto& accessor = model.accessors[primitive.attributes["NORMAL"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					cpuMesh.Normals = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data)+verticesCount);

					{
						GltfLoadTimer<GltfLoadBuffer> T;
						normalBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
						Device->SetDebugName(normalBuffer, (mesh.name + " (Normals)").c_str());
					}
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
					const auto& mat = model.materials[primitive.material];

					int modelAlbedoId = mat.pbrMetallicRoughness.baseColorTexture.index;
					if (modelAlbedoId != -1)
					{
						int albedoId = CreateTexture(model.textures[modelAlbedoId].source, model.textures[modelAlbedoId].name.c_str());
						matid = albedoId;
					}
				}

				Matrix Transform(1); // TODO: global transform
				//Transform = GltfNodes[NodeId].Transform;

				GPUSceneMesh Mesh;
				Mesh.BLAS = BLAS;
				Mesh.Transform = Transform;
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
		Log::Message("Meshes loaded, total time: %0.2f s", MeshTimer.Elapsed());
		Log::Message("Buffer load time: %0.2f s", GltfLoadTimes[GltfLoadBuffer]);
		Log::Message("Material load time: %0.2f s", GltfLoadTimes[GltfLoadMaterial]);

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
			if (!DecalImage.LoadFromFile("./Data/Textures/Detail.dds"))
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
