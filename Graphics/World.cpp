#include "World.h"

#include <Math/Quaternion.h>

#include <limits.h>
#include <float.h>
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
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, EngineWorld* World);

	EngineWorld::EngineWorld()
	{
		Physics.SetGravity(Vector3(0, -9.81f, 0));
	}

	void EngineWorld::LoadLevelGLTF(const char* Path)
	{
		SceneGPU = LoadScene(Device, MainView.CameraCur, Path, this);
	}

	int EngineWorld::LoadMesh(const Model& MeshModel)
	{
		const SubModel& SModel = MeshModel.GetSubModel(0);

		std::vector<CPUMeshResource> Primitives;
		CPUMeshResource& CPUMesh = Primitives.emplace_back();

		{
			for (int i = 0; i < (int)SModel.VerticesCount; i++)
			{
				CPUMesh.Vertices.push_back(SModel.Positions[i]);
				CPUMesh.Normals.push_back(SModel.Normals[i]);
				CPUMesh.UV1.push_back(SModel.UVs[i]);
				CPUMesh.Tangents.push_back(Vector4(SModel.Tangents[i], 1));
			}

			for (int i = 0; i < (int)SModel.IndicesCount; i++)
			{
				CPUMesh.Indices.push_back(SModel.Indices[i]);
			}
		}

		return LoadMesh(Primitives);
	}

	int EngineWorld::LoadMesh(std::span<CPUMeshResource> MeshPrimitives)
	{
		int Id = (int)Meshes.size();
		Meshes.push_back(new Mesh2{});
		Mesh2& Mesh = *Meshes.back();

		Mesh.BoundingBox = Box(Vector3(FLT_MAX), Vector3(-FLT_MAX));

		for (CPUMeshResource& Primitive : MeshPrimitives)
		{
			MeshPrimitive& Prim = Mesh.Primitives.emplace_back();

			Prim.CPU = Primitive;
			CPUMeshResource& CPUMesh = Prim.CPU;
			GPUMeshResource& GPUMesh = Prim.GPU;

			GPUMesh.VertexCount = (int)CPUMesh.Vertices.size();
			GPUMesh.IndicesCount = (int)CPUMesh.Indices.size();

			GPUMesh.Vertices = CreateMeshBuffer(Device, sizeof(Vector3) * CPUMesh.Vertices.size(), true, CPUMesh.Vertices.data());
			GPUMesh.Normals = CreateMeshBuffer(Device, sizeof(Vector3) * CPUMesh.Normals.size(), true, CPUMesh.Normals.data());
			GPUMesh.Tangents = CreateMeshBuffer(Device, sizeof(Vector4) * CPUMesh.Tangents.size(), true, CPUMesh.Tangents.data());
			GPUMesh.UV1 = CreateMeshBuffer(Device, sizeof(Vector2) * CPUMesh.UV1.size(), true, CPUMesh.UV1.data());
			GPUMesh.Indices = CreateMeshBuffer(Device, sizeof(u32) * CPUMesh.Indices.size(), true, CPUMesh.Indices.data());

			// Primitive bounding box
			{
				Vector3 MinVertex(FLT_MAX);
				Vector3 MaxVertex(-FLT_MAX);

				for (int i = 0; i < (int)CPUMesh.Vertices.size(); i++)
				{
					Vector3 Vertex = CPUMesh.Vertices[i];

					MinVertex = Vector3::Min(Vertex, MinVertex);
					MaxVertex = Vector3::Max(Vertex, MaxVertex);
				}


				Prim.BoundingBox = Box(MinVertex, MaxVertex);

				// total mesh bounding box
				Mesh.BoundingBox.Min = Vector3::Min(Mesh.BoundingBox.Min, MinVertex);
				Mesh.BoundingBox.Max = Vector3::Max(Mesh.BoundingBox.Max, MaxVertex);
			}

			// BLAS
			{
				Columbus::AccelerationStructureDesc blasDesc;
				blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
				blasDesc.Vertices = GPUMesh.Vertices;
				blasDesc.Indices = GPUMesh.Indices;
				blasDesc.VerticesCount = GPUMesh.VertexCount;
				blasDesc.IndicesCount = GPUMesh.IndicesCount;
				GPUMesh.BLAS = Device->CreateAccelerationStructure(blasDesc);

				Device->SetDebugName(GPUMesh.BLAS, "Mesh BLAS");
				AddProfilingMemory(MemoryCounter_SceneBLAS, GPUMesh.BLAS->GetSize());
			}
		}

		return Id;
	}

	int EngineWorld::LoadMesh(const char* AssetPath)
	{
		// TODO: wtf is this
		Model* asd = new Model();
		Model& model = *asd;
		model.Load(AssetPath);
		model.RecalculateTangents();

		return LoadMesh(model);
	}

	Sound* EngineWorld::LoadSoundAsset(const char* AssetPath)
	{
		// TODO: don't load already loaded sounds
		Sound* Snd = new Sound();
		if (!Snd->Load(AssetPath))
		{
			delete Snd;
			return nullptr;
		}

		Sounds.push_back(Snd);
		return Snd;
	}

	void EngineWorld::UnloadSoundAsset(Sound* Snd)
	{
		// TODO:
	}

	// TODO: delete
	GameObjectId EngineWorld::CreateGameObject(const char* Name, int Mesh)
	{
		return -1;
	}

	// TODO: delete
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

	HWorldIntersectionResult EngineWorld::CastRayClosestHit(const Geometry::Ray& Ray, float Distance, int CollisionMask)
	{
		return CastRayClosestHit(Ray.Origin, Ray.Origin + Ray.Direction*Distance, CollisionMask);
	}

	HWorldIntersectionResult EngineWorld::CastRayClosestHit(const Vector3& From, const Vector3& To, int CollisionMask)
	{
		const auto btFrom = btVector3(From.X, From.Y, From.Z);
		const auto btTo = btVector3(To.X, To.Y, To.Z);
		btCollisionWorld::ClosestRayResultCallback Callback(btFrom, btTo);
		Callback.m_collisionFilterMask = CollisionMask;
		Physics.mWorld->rayTest(btFrom, btTo, Callback);

		const auto btN = Callback.m_hitNormalWorld;
		const auto btP = Callback.m_hitPointWorld;

		HWorldIntersectionResult Result;
		Result.bHasIntersection = Callback.hasHit();
		Result.IntersectionFraction = Callback.m_closestHitFraction;
		Result.IntersectionPoint  = Vector3(btP.x(), btP.y(), btP.z());
		Result.IntersectionNormal = Vector3(btN.x(), btN.y(), btN.z());
		Result.HitThing = Callback.m_collisionObject ? (AThing*)Callback.m_collisionObject->getUserPointer() : nullptr;

		return Result;
	}

	HWorldIntersectionResult EngineWorld::CastCameraRayClosestHit(const Vector2& NormalisedScreenCoordinates, float MaxDistance, int CollisionMask)
	{
		Vector2 NDC = NormalisedScreenCoordinates * 2 - 1;
		NDC.Y = -NDC.Y;

		Geometry::Ray CameraRay{
			.Origin = MainView.CameraCur.Pos,
			.Direction = MainView.CameraCur.CalcRayByNdc(NDC)
		};

		return CastRayClosestHit(CameraRay, MaxDistance, CollisionMask);
	}

	HStableThingId EngineWorld::AddThing(AThing* Thing)
	{
		HStableThingId Id = AllThings.Add(Thing);
		Thing->StableId = Id;
		return Id;
	}

	void EngineWorld::DeleteThing(HStableThingId ThingId)
	{
		AThing* Thing = *AllThings.Get(ThingId);
		Thing->OnDestroy();
		AllThings.Remove(ThingId);
		delete Thing;
	}

	void EngineWorld::BeginFrame()
	{
		ResetProfiling();

		MainView.CameraPrev = MainView.CameraCur;
		MainView.DebugRender.Clear();
	}

	void EngineWorld::Update(float DeltaTime)
	{
		GlobalTime += DeltaTime;

		UpdateTransforms();
		SceneGPU->Sky = Sky;
		MainView.UI = &UI;

		Physics.Step(DeltaTime, 1);
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

			while (ParentId != -1 && Object.Id != Object.ParentId)
			{
				GameObject& ParentObj = GameObjects[ParentId];
				GlobalTransform = ParentObj.Trans.GetMatrix() * GlobalTransform;
				ParentId = ParentObj.ParentId;

				if (Object.ParentId == ParentObj.ParentId)
					break;
			}

			/*if (Object.MeshId != -1)
			{
				for (int Prim : Object.GPUScenePrimitives)
				{
					SceneGPU->Meshes[Prim].Transform = GlobalTransform;
				}
			}*/
		}
	}

	void EngineWorld::PostUpdate()
	{
		for (int i = 0; i < (int)AllThings.Size(); i++)
		{
			AThing* Thing = AllThings.Data()[i];
			if (Thing->bRenderStateDirty)
			{
				Thing->OnUpdateRenderState();
				Thing->bRenderStateDirty = false;
			}
		}
	}

	void EngineWorld::EndFrame()
	{
	}

	void EngineWorld::FreeResources()
	{
		SceneGPU = nullptr;

		// mesh unloading
		for (Mesh2* Mesh : Meshes)
		{
			for (MeshPrimitive& Primitive : Mesh->Primitives)
			{
				Device->DestroyBuffer(Primitive.GPU.Vertices);
				Device->DestroyBuffer(Primitive.GPU.Indices);
				Device->DestroyBuffer(Primitive.GPU.UV1);
				Device->DestroyBuffer(Primitive.GPU.UV2);
				Device->DestroyBuffer(Primitive.GPU.Normals);
				Device->DestroyBuffer(Primitive.GPU.Tangents);
				Device->DestroyAccelerationStructure(Primitive.GPU.BLAS);
			}
		}

		Meshes.clear();
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
	SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, EngineWorld* World)
	{
		const tinygltf::LoadImageDataFunction LoadImageFn = [](tinygltf::Image* Img, const int ImgId, std::string* err, std::string* warn,
			int req_width, int req_height, const unsigned char* bytes, int size, void* user_data) -> bool
		{
			u32 W, H, D, Mips;
			TextureFormat Format;
			ImageType Type;

			DataStream Stream = DataStream::CreateFromMemory((u8*)bytes, size);

			u8* Data = nullptr;
			if (!ImageUtils::ImageLoadFromStream(Stream, W, H, D, Mips, Format, Type, Data))
			{
				delete[] Data;
				return false;
			}

			TextureFormatInfo FormatInfo = TextureFormatGetInfo(Format);

			u64 Size = size_t(W * H) * size_t(FormatInfo.BitsPerPixel) / 8;

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

			for (auto& Decal : Scene->Decals)
			{
				Device->DestroyTexture(Decal.Texture);
			}

			GPUScene::DestroyGPUScene(Scene, Device);
		});
		World->SceneGPU = Scene;

		std::unordered_map<int, int> LoadedTextures;

		auto CreateTexture = [Scene, Device, &model, &LoadedTextures](int textureId, const char* name, bool srgb) -> int
		{
			if (LoadedTextures.contains(textureId))
			{
				return LoadedTextures[textureId];
			}

			GltfLoadTimer<GltfLoadMaterial> T;

			auto& image = model.images[textureId];
			TextureFormat format = (TextureFormat)image.pixel_type;

			u64 TextureDataSize = image.image.size();
			u8* TextureData = image.image.data();
			UPtr<u8> ConvertedData;

			TextureFormatInfo formatInfo = TextureFormatGetInfo(format);
			
			if (!formatInfo.HasCompression)
			{
				Log::Warning("[Level Loading] Uncompressed texture %s", image.name.c_str());
			}

			// TODO: proper mechanism
			if (format == TextureFormat::RGBA8 && srgb)
				format = TextureFormat::RGBA8SRGB;

			// GPUs don't support RGB, so convert to RGBA instead
			if (format == TextureFormat::RGB8)
			{
				Log::Warning("Applying RGB8->RGBA8 convertion to texture %s", image.name.c_str());

				TextureDataSize = image.width * image.height * 4;
				ConvertedData = UPtr<u8>(new u8[TextureDataSize]); // TODO: how to manage allocations in the loading system properly?

				for (u64 pixel = 0; pixel < image.width * image.height; pixel++)
				{
					ConvertedData.get()[pixel * 4 + 0] = TextureData[pixel * 3 + 0];
					ConvertedData.get()[pixel * 4 + 1] = TextureData[pixel * 3 + 1];
					ConvertedData.get()[pixel * 4 + 2] = TextureData[pixel * 3 + 2];
					ConvertedData.get()[pixel * 4 + 3] = 255; // alpha to 1
				}

				TextureData = ConvertedData.get();

				format = srgb ? TextureFormat::RGBA8SRGB : TextureFormat::RGBA8;
			}

			Image img;
			img.Format = format;
			img.Width = image.width;
			img.Height = image.height;
			img.Size = TextureDataSize;
			img.Data = TextureData;
			img.MipMaps = 1; // TODO: load all mip maps

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

		// materials
		{
			for (int i = 0; i < model.materials.size(); i++)
			{
				tinygltf::Material& Mat = model.materials[i];

				Material Result;

				int AlbedoId = Mat.pbrMetallicRoughness.baseColorTexture.index;
				if (AlbedoId != -1)
				{
					Result.AlbedoId = CreateTexture(model.textures[AlbedoId].source, model.textures[AlbedoId].name.c_str(), true);
				}

				int NormalId = Mat.normalTexture.index;
				if (NormalId != -1)
				{
					Result.NormalId = CreateTexture(model.textures[NormalId].source, model.textures[NormalId].name.c_str(), false);
				}

				int RoughnessMetallicId = Mat.pbrMetallicRoughness.metallicRoughnessTexture.index;
				if (RoughnessMetallicId != -1)
				{
					// TODO: convert separate Occlusion texture into ORM
					Result.OrmId = CreateTexture(model.textures[RoughnessMetallicId].source, model.textures[RoughnessMetallicId].name.c_str(), false);
				}

				int EmissiveId = Mat.emissiveTexture.index;
				if (EmissiveId != -1)
				{
					Result.EmissiveId = CreateTexture(model.textures[EmissiveId].source, model.textures[EmissiveId].name.c_str(), false);
				}

				Result.AlbedoFactor = Vector4(Mat.pbrMetallicRoughness.baseColorFactor[0], Mat.pbrMetallicRoughness.baseColorFactor[1], Mat.pbrMetallicRoughness.baseColorFactor[2], Mat.pbrMetallicRoughness.baseColorFactor[3]);
				Result.EmissiveFactor = Vector4(Mat.emissiveFactor[0], Mat.emissiveFactor[1], Mat.emissiveFactor[2], 1);

				// TODO:
				Result.Roughness = 1;
				Result.Metallic = 0;

				Scene->Materials.push_back(Result);
			}
		}

		// meshes
		for (int i = 0; i < (int)model.meshes.size(); i++)
		{
			tinygltf::Mesh& mesh = model.meshes[i];

			std::vector<CPUMeshResource> Primitives;

			for (auto& primitive : mesh.primitives)
			{
				CPUMeshResource& CPUMesh = Primitives.emplace_back();

				u32 indicesCount;
				u32 verticesCount;

				// Indices
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

					CPUMesh.Indices = indices;
				}

				// Vertices
				{
					const auto& accessor = model.accessors[primitive.attributes["POSITION"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					CPUMesh.Vertices = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data)+verticesCount);
				}

				// UV1
				if (primitive.attributes.contains("TEXCOORD_0"))
				{
					const auto& accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					CPUMesh.UV1 = std::vector<Vector2>((Vector2*)(data), (Vector2*)(data)+verticesCount);
				}
				else
				{
					Log::Warning("Mesh %s doesn't have UV1", mesh.name.c_str());

					verticesCount = CPUMesh.Vertices.size();
					CPUMesh.UV1 = std::vector<Vector2>(CPUMesh.Vertices.size(), Vector2());
				}

				// Normals
				{
					const auto& accessor = model.accessors[primitive.attributes["NORMAL"]];
					const auto& view = model.bufferViews[accessor.bufferView];
					const auto& buffer = model.buffers[view.buffer];
					const auto& offset = accessor.byteOffset + view.byteOffset;
					const void* data = buffer.data.data() + offset;
					verticesCount = accessor.count;

					CPUMesh.Normals = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data)+verticesCount);
				}

				// Tangents
				{
					CPUMesh.CalculateTangents();
				}
			}

			int MeshId = World->LoadMesh(Primitives);
		}

		// scene graph
		{
			// get nodes and transforms
			for (int i = 0; i < model.nodes.size(); i++)
			{
				tinygltf::Node& Node = model.nodes[i];
				Transform Trans;

				if (Node.matrix.size() > 0)
				{
					Matrix LocalTransform(1);
					// GLTF matrices are column-major
					double* GltfMatrix = Node.matrix.data();
					for (int i = 0; i < 4; i++)
					{
						Vector4 Column((float)GltfMatrix[i * 4 + 0], (float)GltfMatrix[i * 4 + 1], (float)GltfMatrix[i * 4 + 2], (float)GltfMatrix[i * 4 + 3]);
						LocalTransform.SetColumn(i, Column);
					}

					Vector3 T, R, S;
					LocalTransform.DecomposeTransform(T, R, S);

					Trans.Position = T;
					Trans.Rotation = Quaternion(R);
					Trans.Scale = S;
				}
				else
				{
					if (Node.scale.size() > 0)
					{
						Trans.Scale = Vector3(Node.scale[0], Node.scale[1], Node.scale[2]);
					}

					if (Node.rotation.size() > 0)
					{
						// negate imaginary components to rotate it into a proper basis
						Quaternion Quat(-Node.rotation[0], -Node.rotation[1], -Node.rotation[2], Node.rotation[3]);
						//Quaternion Quat(Node.rotation[0], Node.rotation[1], Node.rotation[2], Node.rotation[3]);

						Trans.Rotation = Quat;
					}

					if (Node.translation.size() > 0)
					{
						Trans.Position = Vector3(Node.translation[0], Node.translation[1], Node.translation[2]);
					}
				}

				Trans.Update();

				if (Node.extensions.contains("KHR_lights_punctual"))
				{
					int light = Node.extensions["KHR_lights_punctual"].Get("light").GetNumberAsInt();
					const auto& gltfLight = model.lights[light];

					ALight* LightThing = new ALight();
					LightThing->World = World;
					LightThing->Trans = Trans;
					LightThing->Name = Node.name;

					if (gltfLight.type == "point")
						LightThing->Type = LightType::Point;
					if (gltfLight.type == "directional")
						LightThing->Type = LightType::Directional;

					LightThing->Colour.X = gltfLight.color[0];
					LightThing->Colour.Y = gltfLight.color[1];
					LightThing->Colour.Z = gltfLight.color[2];
					LightThing->Range = gltfLight.range;
					LightThing->Energy = 10.0f;
					LightThing->Shadows = true;

					if (LightThing->Range < 0.01f)
						LightThing->Range = 10.0f;

					World->AddThing(LightThing);
					LightThing->OnCreate();

					continue;
				}

				if (Node.mesh == -1) continue;

				{
					AMeshInstance* MeshThing = new AMeshInstance();
					MeshThing->World = World;
					MeshThing->Trans = Trans;
					MeshThing->Name = Node.name;
					MeshThing->MeshID = Node.mesh;

					for (int prim = 0; prim < (int)model.meshes[Node.mesh].primitives.size(); prim++)
					{
						auto& primitive = model.meshes[Node.mesh].primitives[prim];
						int MaterialId = primitive.material > -1 ? primitive.material : -1;
						MeshThing->Materials.push_back(MaterialId);
					}

					World->AddThing(MeshThing);
					MeshThing->OnCreate();

					continue;
				}
			}

			// TODO: compute parents
			/*for (int i = 0; i < World->GameObjects.size(); i++)
			{
				GameObject& Object = World->GameObjects[i];
				for (int Child : Object.Children)
				{
					World->GameObjects[Child].ParentId = i;
				}
			}*/

		}

		Log::Message("Meshes loaded, total time: %0.2f s", MeshTimer.Elapsed());
		Log::Message("Buffer load time: %0.2f s", GltfLoadTimes[GltfLoadBuffer]);
		Log::Message("Material load time: %0.2f s", GltfLoadTimes[GltfLoadMaterial]);

		// create empty TLAS of MaxMeshes size
		{
			// TLAS and BLASes should be packed into GPU scene
			AccelerationStructureDesc TlasDesc;
			TlasDesc.Type = AccelerationStructureType::TLAS;
			TlasDesc.Instances = {};
			for (int i = 0; i < GPUScene::MaxMeshes; i++)
			{
				TlasDesc.Instances.push_back({ Matrix(1), nullptr });
			}

			Scene->TLAS = Device->CreateAccelerationStructure(TlasDesc);
		}

		Scene->MainCamera = GPUCamera(DefaultCamera);

		if (Device->SupportsRayTracing())
		{
			AddProfilingMemory(MemoryCounter_SceneTLAS, Scene->TLAS->GetSize());
		}

		if (0)
		{
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
		}

		return Scene;
	}
}


// Things implementation
namespace Columbus
{

	void ALight::OnCreate()
	{
		Super::OnCreate();
		OnUpdateRenderState();
	}

	void ALight::OnUpdateRenderState()
	{
		Trans.Update();
		const auto RotMat = Trans.Rotation.ToMatrix();

		GPULight GL{};
		GL.Color = Vector4(Colour * Energy, 1);
		GL.Position = Vector4(Trans.Position, 1);
		GL.Direction = (Vector4(1, 0, 0, 1) * RotMat).Normalized();
		GL.Range = Range;
		GL.SourceRadius = SourceRadius;
		GL.Type = Type;

		if (Shadows)
			GL.Flags = GL.Flags | ELightFlags::Shadow;
		
		switch (Type)
		{
		case LightType::Spot:
		{
			float inner = cosf(Math::Radians(InnerAngle));
			float outer = cosf(Math::Radians(OuterAngle));

			GL.SizeOrSpotAngles = Vector2(inner, outer);
		}
			break;
		case LightType::Rectangle:
			GL.SizeOrSpotAngles = Size * 0.5f;
			if (TwoSided)
				GL.Flags = GL.Flags | ELightFlags::TwoSided;
			break;
		case LightType::Line:
			GL.SizeOrSpotAngles = Vector2(Length, 0);
			break;
		}

		if (GPULight* LightProxy = World->SceneGPU->Lights.Get(LightHandle))
		{
			*LightProxy = GL;
		}
		else
		{
			LightHandle = World->SceneGPU->Lights.Add(GL);
		}
	}

	void ALight::OnDestroy()
	{
		World->SceneGPU->Lights.Remove(LightHandle);

		Super::OnDestroy();
	}

	void AMeshInstance::OnCreate()
	{
		Super::OnCreate();
		assert(World != nullptr);
		assert(MeshID != -1);

		// TODO: make adding these mesh proxies easier!

		int i = 0;
		for (MeshPrimitive& Prim : World->Meshes[MeshID]->Primitives)
		{
			GPUSceneMesh GPUMesh;
			GPUMesh.MeshResource = &Prim.GPU;
			GPUMesh.Transform = Trans.GetMatrix();

			if (Materials.size() > i)
			{
				World->SceneGPU->Materials[i].Roughness = 0.2f;
				GPUMesh.MaterialId = Materials[i];
			}

			MeshPrimitives.push_back(World->SceneGPU->Meshes.Add(GPUMesh));
		}

		// TODO: collision proxy setup
		{
			const Mesh2& mesh = *World->Meshes[MeshID];

			for (int j = 0; j < (int)mesh.Primitives.size(); j++)
			{
				int numFaces = mesh.Primitives[j].CPU.Indices.size() / 3;
				int vertStride = sizeof(Vector3);
				int indexStride = 3 * sizeof(u32);

				btTriangleIndexVertexArray* va = new btTriangleIndexVertexArray(numFaces,
					(int*)mesh.Primitives[j].CPU.Indices.data(),
					indexStride,
					mesh.Primitives[j].CPU.Vertices.size(), (btScalar*)mesh.Primitives[j].CPU.Vertices.data(), vertStride);
				btBvhTriangleMeshShape* triShape = new btBvhTriangleMeshShape(va, true);

				Rigidbody* MeshRB = new Rigidbody(triShape);
				MeshRB->SetTransform(Trans);
				MeshRB->SetStatic(true);
				MeshRB->mRigidbody->setUserPointer(this);

				World->Physics.AddRigidbody(MeshRB);

				Rigidbodies.push_back(MeshRB);
			}
		}
	}

	void AMeshInstance::OnUpdateRenderState()
	{
		Trans.Update();

		for (HStableMeshId Mesh : MeshPrimitives)
		{
			GPUSceneMesh* Proxy = World->SceneGPU->Meshes.Get(Mesh);
			if (Proxy)
			{
				Proxy->Transform = Trans.GetMatrix();
			}
		}

		for (Rigidbody* RB : Rigidbodies)
		{
			RB->SetTransform(Trans);
		}

		Super::OnUpdateRenderState();
	}

	void AMeshInstance::OnDestroy()
	{
		for (HStableMeshId Mesh : MeshPrimitives)
		{
			World->SceneGPU->Meshes.Remove(Mesh);
		}
		MeshPrimitives.clear();

		for (Rigidbody* RB : Rigidbodies)
		{
			World->Physics.RemoveRigidbody(RB);
			delete RB;
		}
		Rigidbodies.clear();

		// TODO: decrement mesh reference count

		Super::OnDestroy();
	}
}

// reflection stuff
using namespace Columbus;

CREFLECT_ENUM_BEGIN(LightType, "")
	CREFLECT_ENUM_FIELD(LightType::Directional, 0)
	CREFLECT_ENUM_FIELD(LightType::Point, 1)
	CREFLECT_ENUM_FIELD(LightType::Spot, 2)
	CREFLECT_ENUM_FIELD(LightType::Rectangle, 3)
	CREFLECT_ENUM_FIELD(LightType::Disc, 4)
	CREFLECT_ENUM_FIELD(LightType::Line, 5)
CREFLECT_ENUM_END()

CREFLECT_DEFINE_VIRTUAL(AThing);
CREFLECT_STRUCT_BEGIN(AThing, "")
	CREFLECT_STRUCT_FIELD(Transform, Trans, "")
	CREFLECT_STRUCT_FIELD(std::string, Name, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ALight);
CREFLECT_STRUCT_BEGIN(ALight, "")
	CREFLECT_STRUCT_FIELD(LightType, Type, "")

	CREFLECT_STRUCT_FIELD(Vector3,   Colour, "Colour")
	CREFLECT_STRUCT_FIELD(float, Energy, "")
	CREFLECT_STRUCT_FIELD(float, Range, "")
	CREFLECT_STRUCT_FIELD(float, SourceRadius, "")
	CREFLECT_STRUCT_FIELD(bool, Shadows, "")

	CREFLECT_STRUCT_FIELD(float, InnerAngle, "")
	CREFLECT_STRUCT_FIELD(float, OuterAngle, "")

	CREFLECT_STRUCT_FIELD(Vector2, Size, "")
	CREFLECT_STRUCT_FIELD(bool, TwoSided, "")

	CREFLECT_STRUCT_FIELD(float, Length, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(ADecal);
CREFLECT_STRUCT_BEGIN(ADecal, "")
CREFLECT_STRUCT_END()

CREFLECT_DEFINE_VIRTUAL(AMeshInstance);
CREFLECT_STRUCT_BEGIN(AMeshInstance, "")
CREFLECT_STRUCT_END()
