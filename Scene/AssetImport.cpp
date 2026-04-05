#include "AssetImport.h"

#include "Core/Asset.h"
#include "Core/Core.h"
#include "Core/Serialisation.h"
#include "Graphics/World.h"
#include "Scene/MeshAsset.h"
#include "Scene/TextureAsset.h"
#include "System/Log.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <span>
#include <unordered_map>
#include <unordered_set>

#include <Lib/tinygltf/tiny_gltf.h>

namespace Columbus::Assets
{

	namespace
	{
		enum class EGltfTextureUsage
		{
			BaseColor,
			Normal,
			Orm,
			Emissive,
		};

		struct HGltfImportPaths
		{
			std::filesystem::path OutputDirectory;
			std::filesystem::path TexturesDirectory;
			std::filesystem::path MaterialsDirectory;
			std::filesystem::path MeshesDirectory;
			std::filesystem::path LevelPath;
			std::string LevelBaseName;
		};

		static std::string ToLowerCopy(std::string Value)
		{
			std::transform(Value.begin(), Value.end(), Value.begin(), [](unsigned char C) { return (char)tolower(C); });
			return Value;
		}

		static std::string NormalisePathSeparators(std::string Path)
		{
			std::replace(Path.begin(), Path.end(), '\\', '/');
			return Path;
		}

		static std::string SanitiseAssetName(std::string Name, const char* Fallback)
		{
			for (char& C : Name)
			{
				const bool bAllowed =
					(C >= 'a' && C <= 'z') ||
					(C >= 'A' && C <= 'Z') ||
					(C >= '0' && C <= '9') ||
					C == '_' || C == '-' || C == '.';
				if (!bAllowed)
					C = '_';
			}

			while (!Name.empty() && (Name.back() == '_' || Name.back() == '.' || Name.back() == ' '))
				Name.pop_back();

			while (!Name.empty() && (Name.front() == '_' || Name.front() == '.' || Name.front() == ' '))
				Name.erase(Name.begin());

			if (Name.empty())
				Name = Fallback;

			return Name;
		}

		static std::filesystem::path MakeUniqueFilePath(
			const std::filesystem::path& Directory,
			const std::string& BaseName,
			const char* Extension,
			std::unordered_set<std::string>& UsedRelativePaths)
		{
			const std::string SanitizedBase = SanitiseAssetName(BaseName, "Asset");

			for (int Suffix = 0; ; Suffix++)
			{
				const std::string CandidateName = Suffix == 0
					? SanitizedBase + Extension
					: SanitizedBase + "_" + std::to_string(Suffix) + Extension;
				const std::string CandidateKey = ToLowerCopy(NormalisePathSeparators((Directory / CandidateName).string()));

				if (!UsedRelativePaths.contains(CandidateKey))
				{
					UsedRelativePaths.insert(CandidateKey);
					return Directory / CandidateName;
				}
			}
		}

		static HGltfImportPaths BuildImportPaths(const char* SourcePath, const char* ImportPath)
		{
			const std::filesystem::path Source = std::filesystem::absolute(SourcePath);
			const std::filesystem::path Requested = std::filesystem::absolute(ImportPath);

			HGltfImportPaths Result;
			if (Requested.has_extension())
			{
				Result.OutputDirectory = Requested.parent_path();
				Result.LevelBaseName = Requested.stem().string();
			}
			else
			{
				Result.OutputDirectory = Requested;
				Result.LevelBaseName = Source.stem().string();
			}

			Result.LevelBaseName = SanitiseAssetName(Result.LevelBaseName, "Level");
			Result.TexturesDirectory = Result.OutputDirectory / "Textures";
			Result.MaterialsDirectory = Result.OutputDirectory / "Materials";
			Result.MeshesDirectory = Result.OutputDirectory / "Meshes";
			Result.LevelPath = Result.OutputDirectory / (Result.LevelBaseName + ".clvl");
			return Result;
		}

		static bool IsDataUri(const std::string& Value)
		{
			return Value.rfind("data:", 0) == 0;
		}

		static std::string MakeExternalImageSourcePath(const std::filesystem::path& SourceAssetPath, const tinygltf::Image& Image, int ImageIndex)
		{
			if (!Image.uri.empty() && !IsDataUri(Image.uri))
			{
				return NormalisePathSeparators((SourceAssetPath.parent_path() / std::filesystem::path(Image.uri)).lexically_normal().string());
			}

			const std::string Name = !Image.name.empty()
				? Image.name
				: ("Image_" + std::to_string(ImageIndex));

			return NormalisePathSeparators(SourceAssetPath.string()) + "#" + Name;
		}

		static const unsigned char* GetAccessorData(const tinygltf::Model& Model, const tinygltf::Accessor& Accessor)
		{
			const tinygltf::BufferView& View = Model.bufferViews[Accessor.bufferView];
			const tinygltf::Buffer& Buffer = Model.buffers[View.buffer];
			return Buffer.data.data() + View.byteOffset + Accessor.byteOffset;
		}

		static int GetAccessorStride(const tinygltf::Model& Model, const tinygltf::Accessor& Accessor)
		{
			const tinygltf::BufferView& View = Model.bufferViews[Accessor.bufferView];
			const int Stride = Accessor.ByteStride(View);
			return Stride > 0 ? Stride : 0;
		}

		static bool ReadIndicesAccessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<u32>& OutIndices)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)tinygltf::GetComponentSizeInBytes(Accessor.componentType);

			OutIndices.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const unsigned char* Element = Data + Index * ElementStride;
				switch (Accessor.componentType)
				{
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					OutIndices[Index] = *(const u8*)Element;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					OutIndices[Index] = *(const u16*)Element;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					OutIndices[Index] = *(const u32*)Element;
					break;
				default:
					return false;
				}
			}

			return true;
		}

		static bool ReadVector2Accessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<Vector2>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC2)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float) * 2;

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const float* Element = (const float*)(Data + Index * ElementStride);
				OutValues[Index] = Vector2(Element[0], Element[1]);
			}

			return true;
		}

		static bool ReadVector3Accessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<Vector3>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC3)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float) * 3;

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const float* Element = (const float*)(Data + Index * ElementStride);
				OutValues[Index] = Vector3(Element[0], Element[1], Element[2]);
			}

			return true;
		}

		static bool ReadVector4Accessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<Vector4>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_VEC4)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float) * 4;

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const float* Element = (const float*)(Data + Index * ElementStride);
				OutValues[Index] = Vector4(Element[0], Element[1], Element[2], Element[3]);
			}

			return true;
		}

		static bool ReadIVector4Accessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<iVector4>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.type != TINYGLTF_TYPE_VEC4)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ComponentSize = (int)tinygltf::GetComponentSizeInBytes(Accessor.componentType);
			const int ElementStride = Stride > 0 ? Stride : ComponentSize * 4;

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const unsigned char* Element = Data + Index * ElementStride;
				iVector4 Value(0, 0, 0, 0);

				for (int Component = 0; Component < 4; Component++)
				{
					const unsigned char* ComponentPtr = Element + Component * ComponentSize;
					int ComponentValue = 0;
					switch (Accessor.componentType)
					{
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						ComponentValue = *(const u8*)ComponentPtr;
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						ComponentValue = *(const u16*)ComponentPtr;
						break;
					default:
						return false;
					}

					switch (Component)
					{
					case 0: Value.X = ComponentValue; break;
					case 1: Value.Y = ComponentValue; break;
					case 2: Value.Z = ComponentValue; break;
					case 3: Value.W = ComponentValue; break;
					}
				}

				OutValues[Index] = Value;
			}

			return true;
		}

		static Transform BuildNodeLocalTransform(const tinygltf::Node& Node)
		{
			Transform Result;

			if (!Node.matrix.empty())
			{
				Matrix LocalMatrix(1.0f);
				const double* GltfMatrix = Node.matrix.data();
				for (int Column = 0; Column < 4; Column++)
				{
					LocalMatrix.SetColumn(Column, Vector4(
						(float)GltfMatrix[Column * 4 + 0],
						(float)GltfMatrix[Column * 4 + 1],
						(float)GltfMatrix[Column * 4 + 2],
						(float)GltfMatrix[Column * 4 + 3]));
				}

				Result.SetFromMatrix(LocalMatrix);
				return Result;
			}

			if (!Node.scale.empty())
				Result.Scale = Vector3((float)Node.scale[0], (float)Node.scale[1], (float)Node.scale[2]);

			if (!Node.rotation.empty())
				Result.Rotation = Quaternion((float)-Node.rotation[0], (float)-Node.rotation[1], (float)-Node.rotation[2], (float)Node.rotation[3]);

			if (!Node.translation.empty())
				Result.Position = Vector3((float)Node.translation[0], (float)Node.translation[1], (float)Node.translation[2]);

			Result.Update();
			return Result;
		}

		static void ComputeNodeWorldTransformRecursive(
			int NodeIndex,
			const std::vector<int>& ParentIndices,
			const tinygltf::Model& Model,
			std::vector<Transform>& OutWorldTransforms,
			std::vector<bool>& Visited)
		{
			if (Visited[NodeIndex])
				return;

			const Transform LocalTransform = BuildNodeLocalTransform(Model.nodes[NodeIndex]);
			if (ParentIndices[NodeIndex] >= 0)
			{
				const int ParentIndex = ParentIndices[NodeIndex];
				ComputeNodeWorldTransformRecursive(ParentIndex, ParentIndices, Model, OutWorldTransforms, Visited);

				const Matrix WorldMatrix = OutWorldTransforms[ParentIndex].GetMatrix() * LocalTransform.GetMatrix();
				OutWorldTransforms[NodeIndex].SetFromMatrix(WorldMatrix);
			}
			else
			{
				OutWorldTransforms[NodeIndex] = LocalTransform;
			}

			Visited[NodeIndex] = true;
		}

		static void SaveJsonToFile(const nlohmann::json& Json, const std::filesystem::path& Path)
		{
			std::filesystem::create_directories(Path.parent_path());
			std::ofstream Stream(Path);
			Stream << std::setw(4) << Json;
		}

		static bool SaveMaterialAssetToFile(const Material& MaterialAsset, const std::filesystem::path& Path)
		{
			nlohmann::json Json;
			Reflection_SerialiseStructJson(const_cast<Material&>(MaterialAsset), Json);
			SaveJsonToFile(Json, Path);
			return true;
		}

		static bool SaveLevelAssetToFile(const HLevel& LevelAsset, const std::filesystem::path& Path)
		{
			nlohmann::json Json;
			HLevel LevelMetadata;
			LevelMetadata.EffectsSettings = LevelAsset.EffectsSettings;
			Reflection_SerialiseStructJson(LevelMetadata, Json);
			Json["things"] = nlohmann::json::array();

			for (AThing* Thing : LevelAsset.Things)
			{
				nlohmann::json ThingJson;
				Reflection_SerialiseStructJson<AThing>(*Thing, ThingJson);
				ThingJson["1_Guid"] = (u64)Thing->Guid;
				Json["things"].push_back(ThingJson);
			}

			SaveJsonToFile(Json, Path);
			return true;
		}

		static bool LoadGltfWithDecodedImages(const char* SourcePath, tinygltf::Model& OutModel, std::unordered_map<int, Image>& OutImages)
		{
			const tinygltf::LoadImageDataFunction LoadImageFn =
				[](tinygltf::Image* Img, const int ImgId, std::string* Err, std::string* Warn,
					int ReqWidth, int ReqHeight, const unsigned char* Bytes, int Size, void* UserData) -> bool
				{
					auto& Images = *static_cast<std::unordered_map<int, Image>*>(UserData);
					Image& DecodedImage = Images[ImgId];

					DataStream Stream = DataStream::CreateFromMemory((u8*)Bytes, Size);
					if (!DecodedImage.LoadFromStream(Stream))
					{
						Images.erase(ImgId);
						return false;
					}

					Img->image.resize(1);
					return true;
				};

			tinygltf::TinyGLTF Loader;
			std::string Err;
			std::string Warn;
			Loader.SetImageLoader(LoadImageFn, &OutImages);

			const std::string Extension = ToLowerCopy(std::filesystem::path(SourcePath).extension().string());
			const bool bLoaded = Extension == ".glb"
				? Loader.LoadBinaryFromFile(&OutModel, &Err, &Warn, SourcePath)
				: Loader.LoadASCIIFromFile(&OutModel, &Err, &Warn, SourcePath);

			if (!Warn.empty())
				Log::Warning("[ImportLevel] %s", Warn.c_str());

			if (!bLoaded)
			{
				Log::Error("[ImportLevel] Couldn't load scene %s: %s", SourcePath, Err.c_str());
				return false;
			}

			return true;
		}

		static bool ReadMeshPrimitive(const tinygltf::Model& Model, const tinygltf::Primitive& Primitive, CPUMeshResource& OutMesh)
		{
			if (Primitive.mode != TINYGLTF_MODE_TRIANGLES && Primitive.mode != -1)
			{
				Log::Warning("[ImportLevel] Unsupported glTF primitive mode %d, skipping primitive", Primitive.mode);
				return false;
			}

			if (!Primitive.attributes.contains("POSITION") || Primitive.indices < 0)
				return false;

			if (!ReadIndicesAccessor(Model, Primitive.indices, OutMesh.Indices))
				return false;
			if (!ReadVector3Accessor(Model, Primitive.attributes.at("POSITION"), OutMesh.Vertices))
				return false;

			if (Primitive.attributes.contains("NORMAL"))
				ReadVector3Accessor(Model, Primitive.attributes.at("NORMAL"), OutMesh.Normals);
			if (Primitive.attributes.contains("TANGENT"))
				ReadVector4Accessor(Model, Primitive.attributes.at("TANGENT"), OutMesh.Tangents);
			if (Primitive.attributes.contains("TEXCOORD_0"))
				ReadVector2Accessor(Model, Primitive.attributes.at("TEXCOORD_0"), OutMesh.UV1);
			if (Primitive.attributes.contains("TEXCOORD_1"))
				ReadVector2Accessor(Model, Primitive.attributes.at("TEXCOORD_1"), OutMesh.UV2);
			if (Primitive.attributes.contains("WEIGHTS_0"))
				ReadVector4Accessor(Model, Primitive.attributes.at("WEIGHTS_0"), OutMesh.BoneWeights);
			if (Primitive.attributes.contains("JOINTS_0"))
				ReadIVector4Accessor(Model, Primitive.attributes.at("JOINTS_0"), OutMesh.BoneIndices);

			return true;
		}

		static void ApplyGltfTextureUsageSettings(Texture2& TextureAsset, const Image& SourceImage, EGltfTextureUsage Usage)
		{
			const bool bHasMeaningfulAlpha = ImageUtils::HasMeaningfulAlpha(SourceImage);

			switch (Usage)
			{
			case EGltfTextureUsage::BaseColor:
				TextureAsset.ImportSettings.ColourSpace = ColourSpaceMode::SRGB;
				TextureAsset.ImportSettings.Compression = bHasMeaningfulAlpha ? CompressionMode::BC7 : CompressionMode::BC1;
				TextureAsset.ImportSettings.MipGen = MipGenMode::Default;
				break;
			case EGltfTextureUsage::Normal:
				TextureAsset.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
				TextureAsset.ImportSettings.Compression = CompressionMode::BC5;
				TextureAsset.ImportSettings.MipGen = MipGenMode::NormalMap;
				break;
			case EGltfTextureUsage::Orm:
				TextureAsset.ImportSettings.ColourSpace = ColourSpaceMode::Linear;
				TextureAsset.ImportSettings.Compression = bHasMeaningfulAlpha ? CompressionMode::BC7 : CompressionMode::BC1;
				TextureAsset.ImportSettings.MipGen = MipGenMode::Default;
				break;
			case EGltfTextureUsage::Emissive:
				TextureAsset.ImportSettings.ColourSpace = ColourSpaceMode::SRGB;
				TextureAsset.ImportSettings.Compression = bHasMeaningfulAlpha ? CompressionMode::BC7 : CompressionMode::BC1;
				TextureAsset.ImportSettings.MipGen = MipGenMode::Default;
				break;
			}

			TextureAsset.ImportSettings.MaxSize = 0;
			TextureAsset.ImportSettings.AddressMode = TextureAddressMode::Repeat;
		}

		static const char* GetUsageSuffix(EGltfTextureUsage Usage)
		{
			switch (Usage)
			{
			case EGltfTextureUsage::BaseColor: return "BaseColor";
			case EGltfTextureUsage::Normal: return "Normal";
			case EGltfTextureUsage::Orm: return "Orm";
			case EGltfTextureUsage::Emissive: return "Emissive";
			default: return "Texture";
			}
		}

		struct HGltfTextureKey
		{
			int TextureIndex = -1;
			EGltfTextureUsage Usage = EGltfTextureUsage::BaseColor;

			bool operator==(const HGltfTextureKey&) const = default;
		};

		struct HGltfTextureKeyHash
		{
			size_t operator()(const HGltfTextureKey& Key) const
			{
				return std::hash<int>()(Key.TextureIndex) ^ (std::hash<int>()((int)Key.Usage) << 1);
			}
		};
	}

	void ImportLevel(const char* SourcePath, const char* ImportPath)
	{
		const std::filesystem::path SourceAssetPath = std::filesystem::absolute(SourcePath);
		const HGltfImportPaths OutputPaths = BuildImportPaths(SourcePath, ImportPath);

		std::filesystem::create_directories(OutputPaths.TexturesDirectory);
		std::filesystem::create_directories(OutputPaths.MaterialsDirectory);
		std::filesystem::create_directories(OutputPaths.MeshesDirectory);

		tinygltf::Model Model;
		std::unordered_map<int, Image> DecodedImages;
		if (!LoadGltfWithDecodedImages(SourcePath, Model, DecodedImages))
			return;

		std::unordered_set<std::string> UsedOutputPaths;

		std::unordered_map<HGltfTextureKey, AssetRef<Texture2>, HGltfTextureKeyHash> ImportedTextures;
		std::unordered_map<int, AssetRef<Material>> ImportedMaterials;
		std::unordered_map<int, AssetRef<Mesh2>> ImportedMeshes;

		auto MakeRelativeAssetPath = [](const std::filesystem::path& AbsolutePath) -> std::string
		{
			return AssetSystem::Get().MakePathRelativeToBakedFolder(NormalisePathSeparators(std::filesystem::absolute(AbsolutePath).string()));
		};

		const auto ImportTextureRef = [&](int TextureIndex, EGltfTextureUsage Usage) -> AssetRef<Texture2>
		{
			if (TextureIndex < 0 || TextureIndex >= (int)Model.textures.size())
				return {};

			HGltfTextureKey CacheKey{ TextureIndex, Usage };
			if (ImportedTextures.contains(CacheKey))
				return ImportedTextures[CacheKey];

			const tinygltf::Texture& GltfTexture = Model.textures[TextureIndex];
			if (GltfTexture.source < 0 || !DecodedImages.contains(GltfTexture.source))
				return {};

			const tinygltf::Image& GltfImage = Model.images[GltfTexture.source];
			const Image& SourceImage = DecodedImages[GltfTexture.source];
			const std::string ImageSourcePath = MakeExternalImageSourcePath(SourceAssetPath, GltfImage, GltfTexture.source);

			UPtr<Texture2> TextureAsset(ImportTextureAssetFromImage(SourceImage, ImageSourcePath.c_str()));
			if (!TextureAsset)
				return {};

			ApplyGltfTextureUsageSettings(*TextureAsset, SourceImage, Usage);
			COLUMBUS_ASSERT(RecookTextureAsset(*TextureAsset));

			const std::string TextureBaseName = !GltfTexture.name.empty()
				? GltfTexture.name
				: (!GltfImage.name.empty() ? GltfImage.name : ("Texture_" + std::to_string(TextureIndex)));
			const std::filesystem::path TexturePath = MakeUniqueFilePath(
				OutputPaths.TexturesDirectory,
				TextureBaseName + "_" + GetUsageSuffix(Usage),
				".cas",
				UsedOutputPaths);

			COLUMBUS_ASSERT(SaveTextureAssetToFile(*TextureAsset, TexturePath.string().c_str()));

			AssetRef<Texture2> Ref;
			Ref.Path = MakeRelativeAssetPath(TexturePath);
			ImportedTextures[CacheKey] = Ref;
			return Ref;
		};

		for (int MaterialIndex = 0; MaterialIndex < (int)Model.materials.size(); MaterialIndex++)
		{
			const tinygltf::Material& GltfMaterial = Model.materials[MaterialIndex];
			Material MaterialAsset;

			if (GltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0)
				MaterialAsset.Albedo = ImportTextureRef(GltfMaterial.pbrMetallicRoughness.baseColorTexture.index, EGltfTextureUsage::BaseColor);
			if (GltfMaterial.normalTexture.index >= 0)
				MaterialAsset.Normal = ImportTextureRef(GltfMaterial.normalTexture.index, EGltfTextureUsage::Normal);
			if (GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
				MaterialAsset.Orm = ImportTextureRef(GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index, EGltfTextureUsage::Orm);
			if (GltfMaterial.emissiveTexture.index >= 0)
				MaterialAsset.Emissive = ImportTextureRef(GltfMaterial.emissiveTexture.index, EGltfTextureUsage::Emissive);

			MaterialAsset.AlbedoFactor = Vector4(
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[0],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[2],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[3]);
			MaterialAsset.EmissiveFactor = Vector4(
				(float)GltfMaterial.emissiveFactor[0],
				(float)GltfMaterial.emissiveFactor[1],
				(float)GltfMaterial.emissiveFactor[2],
				1.0f);
			MaterialAsset.Roughness = (float)GltfMaterial.pbrMetallicRoughness.roughnessFactor;
			MaterialAsset.Metallic = (float)GltfMaterial.pbrMetallicRoughness.metallicFactor;

			const std::string MaterialBaseName = !GltfMaterial.name.empty()
				? GltfMaterial.name
				: ("Material_" + std::to_string(MaterialIndex));
			const std::filesystem::path MaterialPath = MakeUniqueFilePath(
				OutputPaths.MaterialsDirectory,
				MaterialBaseName,
				".mat",
				UsedOutputPaths);

			COLUMBUS_ASSERT(SaveMaterialAssetToFile(MaterialAsset, MaterialPath));

			AssetRef<Material> MaterialRef;
			MaterialRef.Path = MakeRelativeAssetPath(MaterialPath);
			ImportedMaterials[MaterialIndex] = MaterialRef;
		}

		for (int MeshIndex = 0; MeshIndex < (int)Model.meshes.size(); MeshIndex++)
		{
			const tinygltf::Mesh& GltfMesh = Model.meshes[MeshIndex];
			std::vector<CPUMeshResource> MeshPrimitives;
			MeshPrimitives.reserve(GltfMesh.primitives.size());

			for (const tinygltf::Primitive& Primitive : GltfMesh.primitives)
			{
				CPUMeshResource CpuMesh;
				if (ReadMeshPrimitive(Model, Primitive, CpuMesh))
					MeshPrimitives.push_back(std::move(CpuMesh));
			}

			UPtr<Mesh2> MeshAsset(CreateMeshAssetFromPrimitives(MeshPrimitives, SourcePath));
			if (!MeshAsset)
				continue;

			for (size_t PrimitiveIndex = 0; PrimitiveIndex < MeshAsset->Primitives.size() && PrimitiveIndex < GltfMesh.primitives.size(); PrimitiveIndex++)
			{
				const tinygltf::Primitive& GltfPrimitive = GltfMesh.primitives[PrimitiveIndex];
				MeshPrimitive& PrimitiveAsset = MeshAsset->Primitives[PrimitiveIndex];

				if (GltfPrimitive.material >= 0 && ImportedMaterials.contains(GltfPrimitive.material))
					PrimitiveAsset.DefaultMaterial = ImportedMaterials[GltfPrimitive.material];

				if (PrimitiveAsset.Name.empty() || PrimitiveAsset.Name.rfind("Primitive ", 0) == 0)
				{
					if (GltfPrimitive.material >= 0 && ImportedMaterials.contains(GltfPrimitive.material) && !Model.materials[GltfPrimitive.material].name.empty())
						PrimitiveAsset.Name = Model.materials[GltfPrimitive.material].name;
					else
						PrimitiveAsset.Name = "Primitive " + std::to_string(PrimitiveIndex);
				}
			}

			const std::string MeshBaseName = !GltfMesh.name.empty()
				? GltfMesh.name
				: ("Mesh_" + std::to_string(MeshIndex));
			const std::filesystem::path MeshPath = MakeUniqueFilePath(
				OutputPaths.MeshesDirectory,
				MeshBaseName,
				".cas",
				UsedOutputPaths);

			COLUMBUS_ASSERT(SaveMeshAssetToFile(*MeshAsset, MeshPath.string().c_str()));

			AssetRef<Mesh2> MeshRef;
			MeshRef.Path = MakeRelativeAssetPath(MeshPath);
			ImportedMeshes[MeshIndex] = MeshRef;
		}

		std::vector<int> ParentIndices(Model.nodes.size(), -1);
		for (int NodeIndex = 0; NodeIndex < (int)Model.nodes.size(); NodeIndex++)
		{
			for (int ChildIndex : Model.nodes[NodeIndex].children)
			{
				if (ChildIndex >= 0 && ChildIndex < (int)ParentIndices.size())
					ParentIndices[ChildIndex] = NodeIndex;
			}
		}

		std::vector<Transform> WorldTransforms(Model.nodes.size());
		std::vector<bool> WorldTransformVisited(Model.nodes.size(), false);

		std::vector<int> RootNodes;
		if (Model.defaultScene >= 0 && Model.defaultScene < (int)Model.scenes.size())
		{
			RootNodes = Model.scenes[Model.defaultScene].nodes;
		}
		else
		{
			for (int NodeIndex = 0; NodeIndex < (int)Model.nodes.size(); NodeIndex++)
			{
				if (ParentIndices[NodeIndex] < 0)
					RootNodes.push_back(NodeIndex);
			}
		}

		for (int RootNode : RootNodes)
		{
			if (RootNode >= 0 && RootNode < (int)Model.nodes.size())
				ComputeNodeWorldTransformRecursive(RootNode, ParentIndices, Model, WorldTransforms, WorldTransformVisited);
		}

		for (int NodeIndex = 0; NodeIndex < (int)Model.nodes.size(); NodeIndex++)
		{
			if (!WorldTransformVisited[NodeIndex])
				ComputeNodeWorldTransformRecursive(NodeIndex, ParentIndices, Model, WorldTransforms, WorldTransformVisited);
		}

		HLevel LevelAsset;
		for (int NodeIndex = 0; NodeIndex < (int)Model.nodes.size(); NodeIndex++)
		{
			const tinygltf::Node& Node = Model.nodes[NodeIndex];
			Transform NodeTransform = WorldTransforms[NodeIndex];
			NodeTransform.Update();

			if (Node.extensions.contains("KHR_lights_punctual"))
			{
				const int LightIndex = Node.extensions.at("KHR_lights_punctual").Get("light").GetNumberAsInt();
				if (LightIndex >= 0 && LightIndex < (int)Model.lights.size())
				{
					const tinygltf::Light& GltfLight = Model.lights[LightIndex];
					ALight* LightThing = new ALight();
					LightThing->Name = !Node.name.empty() ? Node.name : (!GltfLight.name.empty() ? GltfLight.name : ("Light_" + std::to_string(LightIndex)));
					LightThing->Trans = NodeTransform;
					LightThing->Type = GltfLight.type == "directional" ? LightType::Directional : LightType::Point;
					if (GltfLight.type == "spot")
						LightThing->Type = LightType::Spot;
					LightThing->Colour = Vector3((float)GltfLight.color[0], (float)GltfLight.color[1], (float)GltfLight.color[2]);
					LightThing->Range = GltfLight.range > 0.0 ? (float)GltfLight.range : 10.0f;
					LightThing->Energy = GltfLight.intensity > 0.0 ? (float)GltfLight.intensity : 10.0f;
					LightThing->Shadows = true;

					if (GltfLight.type == "spot")
					{
						LightThing->InnerAngle = Math::Degrees((float)GltfLight.spot.innerConeAngle) * 2.0f;
						LightThing->OuterAngle = Math::Degrees((float)GltfLight.spot.outerConeAngle) * 2.0f;
					}

					LevelAsset.Things.push_back(LightThing);
				}
			}

			if (Node.mesh >= 0 && ImportedMeshes.contains(Node.mesh))
			{
				AMeshInstance* MeshThing = new AMeshInstance();
				MeshThing->Name = !Node.name.empty() ? Node.name : (!Model.meshes[Node.mesh].name.empty() ? Model.meshes[Node.mesh].name : ("Mesh_" + std::to_string(Node.mesh)));
				MeshThing->Trans = NodeTransform;
				MeshThing->Mesh = ImportedMeshes[Node.mesh];
				LevelAsset.Things.push_back(MeshThing);
			}
		}

		COLUMBUS_ASSERT(SaveLevelAssetToFile(LevelAsset, OutputPaths.LevelPath));

		for (AThing* Thing : LevelAsset.Things)
			delete Thing;

		Log::Message("[ImportLevel] Imported %s to %s", SourcePath, OutputPaths.LevelPath.string().c_str());
	}

}
