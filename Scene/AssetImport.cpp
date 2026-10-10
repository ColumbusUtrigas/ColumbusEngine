#include "AssetImport.h"

#include "Animation/AnimationAsset.h"
#include "Animation/AnimClip.h"
#include "Animation/Skeleton.h"
#include "Core/Asset.h"
#include "Core/Core.h"
#include "Core/Serialisation.h"
#include "Graphics/World.h"
#include "Scene/MeshAsset.h"
#include "Scene/TextureAsset.h"
#include "System/Log.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
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
			std::filesystem::path SkeletonsDirectory;
			std::filesystem::path AnimationsDirectory;
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
			Result.SkeletonsDirectory = Result.OutputDirectory / "Skeletons";
			Result.AnimationsDirectory = Result.OutputDirectory / "Animations";
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

		static bool ReadFloatAccessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<float>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_SCALAR)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float);

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const float* Element = (const float*)(Data + Index * ElementStride);
				OutValues[Index] = Element[0];
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

		static bool ReadMatrixAccessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<Matrix>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT || Accessor.type != TINYGLTF_TYPE_MAT4)
				return false;

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float) * 16;

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const float* Element = (const float*)(Data + Index * ElementStride);
				Matrix Value(1.0f);
				for (int Column = 0; Column < 4; Column++)
				{
					Value.SetColumn(Column, Vector4(
						Element[Column * 4 + 0],
						Element[Column * 4 + 1],
						Element[Column * 4 + 2],
						Element[Column * 4 + 3]));
				}
				OutValues[Index] = Value;
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

			OutValues.assign(Accessor.count, Vector3(0.0f));

			if (Accessor.bufferView >= 0)
			{
				const unsigned char* Data = GetAccessorData(Model, Accessor);
				const int Stride = GetAccessorStride(Model, Accessor);
				const int ElementStride = Stride > 0 ? Stride : (int)sizeof(float) * 3;

				for (size_t Index = 0; Index < Accessor.count; Index++)
				{
					const float* Element = (const float*)(Data + Index * ElementStride);
					OutValues[Index] = Vector3(Element[0], Element[1], Element[2]);
				}
			}
			else if (!Accessor.sparse.isSparse)
			{
				return false;
			}

			if (Accessor.sparse.isSparse)
			{
				if (Accessor.sparse.indices.bufferView < 0 || Accessor.sparse.indices.bufferView >= (int)Model.bufferViews.size() ||
					Accessor.sparse.values.bufferView < 0 || Accessor.sparse.values.bufferView >= (int)Model.bufferViews.size())
				{
					return false;
				}

				const tinygltf::BufferView& IndicesView = Model.bufferViews[Accessor.sparse.indices.bufferView];
				const tinygltf::BufferView& ValuesView = Model.bufferViews[Accessor.sparse.values.bufferView];
				const tinygltf::Buffer& IndicesBuffer = Model.buffers[IndicesView.buffer];
				const tinygltf::Buffer& ValuesBuffer = Model.buffers[ValuesView.buffer];
				const unsigned char* IndicesData = IndicesBuffer.data.data() + IndicesView.byteOffset + Accessor.sparse.indices.byteOffset;
				const unsigned char* ValuesData = ValuesBuffer.data.data() + ValuesView.byteOffset + Accessor.sparse.values.byteOffset;
				const int IndexComponentSize = (int)tinygltf::GetComponentSizeInBytes(Accessor.sparse.indices.componentType);
				if (IndexComponentSize <= 0)
					return false;

				for (int SparseIndex = 0; SparseIndex < Accessor.sparse.count; SparseIndex++)
				{
					const unsigned char* IndexPtr = IndicesData + SparseIndex * IndexComponentSize;
					size_t ValueIndex = 0;
					switch (Accessor.sparse.indices.componentType)
					{
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:  ValueIndex = *(const u8*)IndexPtr; break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: ValueIndex = *(const u16*)IndexPtr; break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:   ValueIndex = *(const u32*)IndexPtr; break;
					default: return false;
					}

					if (ValueIndex >= OutValues.size())
						continue;

					const float* Element = (const float*)(ValuesData + SparseIndex * sizeof(float) * 3);
					OutValues[ValueIndex] = Vector3(Element[0], Element[1], Element[2]);
				}
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

		static bool ReadWeightsAccessor(const tinygltf::Model& Model, int AccessorIndex, std::vector<Vector4>& OutValues)
		{
			if (AccessorIndex < 0)
				return false;

			const tinygltf::Accessor& Accessor = Model.accessors[AccessorIndex];
			if (Accessor.type != TINYGLTF_TYPE_VEC4)
				return false;

			if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
				return ReadVector4Accessor(Model, AccessorIndex, OutValues);

			const unsigned char* Data = GetAccessorData(Model, Accessor);
			const int Stride = GetAccessorStride(Model, Accessor);
			const int ComponentSize = (int)tinygltf::GetComponentSizeInBytes(Accessor.componentType);
			const int ElementStride = Stride > 0 ? Stride : ComponentSize * 4;

			float Denominator = 1.0f;
			switch (Accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				Denominator = 255.0f;
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				Denominator = 65535.0f;
				break;
			default:
				return false;
			}

			OutValues.resize(Accessor.count);
			for (size_t Index = 0; Index < Accessor.count; Index++)
			{
				const unsigned char* Element = Data + Index * ElementStride;
				float Components[4] = {};

				for (int Component = 0; Component < 4; Component++)
				{
					const unsigned char* ComponentPtr = Element + Component * ComponentSize;
					switch (Accessor.componentType)
					{
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						Components[Component] = (float)(*(const u8*)ComponentPtr) / Denominator;
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						Components[Component] = (float)(*(const u16*)ComponentPtr) / Denominator;
						break;
					}
				}

				OutValues[Index] = Vector4(Components[0], Components[1], Components[2], Components[3]);
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

		static std::string MakeNodeName(const tinygltf::Model& Model, int NodeIndex, const char* Prefix)
		{
			if (NodeIndex >= 0 && NodeIndex < (int)Model.nodes.size() && !Model.nodes[NodeIndex].name.empty())
				return Model.nodes[NodeIndex].name;

			return std::string(Prefix) + "_" + std::to_string(NodeIndex);
		}

		static std::vector<std::string> ReadMorphTargetNames(const tinygltf::Mesh& Mesh)
		{
			std::vector<std::string> Result;
			if (!Mesh.extras.IsObject() || !Mesh.extras.Has("targetNames"))
				return Result;

			const tinygltf::Value& TargetNames = Mesh.extras.Get("targetNames");
			if (!TargetNames.IsArray())
				return Result;

			Result.reserve(TargetNames.ArrayLen());
			for (size_t Index = 0; Index < TargetNames.ArrayLen(); Index++)
			{
				const tinygltf::Value& NameValue = TargetNames.Get((int)Index);
				Result.push_back(NameValue.IsString() ? NameValue.Get<std::string>() : ("Morph_" + std::to_string(Index)));
			}

			return Result;
		}

		static std::vector<float> ReadMorphDefaultWeights(const tinygltf::Mesh& Mesh)
		{
			std::vector<float> Result;
			Result.reserve(Mesh.weights.size());
			for (double Weight : Mesh.weights)
				Result.push_back((float)Weight);
			return Result;
		}

		static std::vector<MeshMorph> BuildMeshMorphs(const tinygltf::Mesh& Mesh)
		{
			const std::vector<std::string> Names = ReadMorphTargetNames(Mesh);
			const std::vector<float> Weights = ReadMorphDefaultWeights(Mesh);

			size_t MorphCount = std::max(Names.size(), Weights.size());
			for (const tinygltf::Primitive& Primitive : Mesh.primitives)
				MorphCount = std::max(MorphCount, Primitive.targets.size());

			for (const tinygltf::Primitive& Primitive : Mesh.primitives)
				assert(Primitive.targets.size() == MorphCount);

			std::vector<MeshMorph> Result;
			Result.reserve(MorphCount);
			for (size_t MorphIndex = 0; MorphIndex < MorphCount; MorphIndex++)
			{
				Result.push_back(MeshMorph
				{
					.Name = MorphIndex < Names.size() && !Names[MorphIndex].empty()
						? Names[MorphIndex]
						: ("Morph_" + std::to_string(MorphIndex)),
					.DefaultWeight = MorphIndex < Weights.size() ? Weights[MorphIndex] : 0.0f,
				});
			}

			return Result;
		}

		static Quaternion GltfQuaternionToEngine(const Vector4& Value)
		{
			Quaternion Result(-Value.X, -Value.Y, -Value.Z, Value.W);
			if (Result.LengthSqr() > 0.000001f)
				Result.Normalize();
			return Result;
		}

		static Quaternion GltfQuaternionTangentToEngine(const Vector4& Value)
		{
			return Quaternion(-Value.X, -Value.Y, -Value.Z, Value.W);
		}

		static EAnimInterpolation GetGltfAnimInterpolation(const std::string& Interpolation)
		{
			if (Interpolation == "CUBICSPLINE")
				return EAnimInterpolation::CubicSpline;
			if (Interpolation == "STEP")
				return EAnimInterpolation::Step;

			return EAnimInterpolation::Linear;
		}

		static AnimJointTrack& FindOrAddJointTrack(AnimClip& Clip, int JointIndex, const std::string& JointName)
		{
			for (AnimJointTrack& Track : Clip.JointTracks)
			{
				if (Track.JointIndex == JointIndex)
					return Track;
			}

			AnimJointTrack& Track = Clip.JointTracks.emplace_back();
			Track.JointIndex = JointIndex;
			Track.JointName = JointName;
			return Track;
		}

		static AnimMorphTrack& FindOrAddMorphTrack(AnimClip& Clip, int MorphIndex, const std::string& MorphName)
		{
			for (AnimMorphTrack& Track : Clip.MorphTracks)
			{
				if (Track.MorphIndex == MorphIndex)
					return Track;
			}

			AnimMorphTrack& Track = Clip.MorphTracks.emplace_back();
			Track.MorphIndex = MorphIndex;
			Track.MorphName = MorphName;
			return Track;
		}

		static bool MorphLayoutsMatch(std::span<const MeshMorph> A, std::span<const MeshMorph> B)
		{
			if (A.size() != B.size())
				return false;

			for (size_t Index = 0; Index < A.size(); Index++)
			{
				if (A[Index].Name != B[Index].Name)
					return false;
			}

			return true;
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

		template <typename T>
		static bool SaveImportedBinaryAsset(const T& Asset, const std::filesystem::path& Path)
		{
			auto Stream = DataStream::CreateFromFile(Path.string().c_str(), "wb");
			if (!Stream.IsValid()) return false;
			const bool Saved = Reflection_SerialiseStructBinary(Stream, const_cast<T&>(Asset)) && Stream.F.Flush();
			const bool Closed = Stream.F.Close();
			return Saved && Closed;
		}

		static void ImportProgress(const char* Stage)
		{
			Log::Message("[ImportLevel] %s", Stage);
			std::fflush(stdout);
		}

		static void RecordImportedFile(HGltfImportResult& Result, const std::filesystem::path& Path)
		{
			Result.Files.push_back(Path.generic_string());
			Log::Message("[ImportLevel] Saved %s", Path.string().c_str());
			std::fflush(stdout);
		}

		static bool SaveJsonToFile(const nlohmann::json& Json, const std::filesystem::path& Path)
		{
			std::filesystem::create_directories(Path.parent_path());
			std::ofstream Stream(Path);
			if (!Stream) return false;
			Stream << std::setw(4) << Json;
			Stream.flush();
			Stream.close();
			return !Stream.fail();
		}

		static bool SaveMaterialAssetToFile(const Material& MaterialAsset, const std::filesystem::path& Path)
		{
			nlohmann::json Json;
			Reflection_SerialiseStructJson(const_cast<Material&>(MaterialAsset), Json);
			return SaveJsonToFile(Json, Path);
		}

		static bool SaveLevelAssetToFile(const HLevel& LevelAsset, const std::filesystem::path& Path)
		{
			nlohmann::json Json;
			HLevel LevelMetadata;
			Reflection_SerialiseStructJson(LevelMetadata, Json);
			Json["things"] = nlohmann::json::array();

			for (AThing* Thing : LevelAsset.Things)
			{
				nlohmann::json ThingJson;
				Reflection_SerialiseStructJson<AThing>(*Thing, ThingJson);
				ThingJson["1_Guid"] = (u64)Thing->Guid;
				Json["things"].push_back(ThingJson);
			}

			return SaveJsonToFile(Json, Path);
		}

		static void ReloadImportedAssetIfLoaded(const std::filesystem::path& AbsolutePath)
		{
			AssetSystem& Assets = AssetSystem::Get();
			const std::string RelativePath = Assets.MakePathRelativeToBakedFolder(NormalisePathSeparators(std::filesystem::absolute(AbsolutePath).string()));
			if (Assets.HasPath(RelativePath))
				Assets.ReloadAsset(RelativePath);
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

		static bool ReadMeshPrimitive(
			const tinygltf::Model& Model,
			const tinygltf::Primitive& Primitive,
			size_t MorphCount,
			CPUMeshResource& OutMesh)
		{
			if (Primitive.mode != TINYGLTF_MODE_TRIANGLES && Primitive.mode != -1)
			{
				Log::Warning("[ImportLevel] Unsupported glTF primitive mode %d, skipping primitive", Primitive.mode);
				return false;
			}

			if (!Primitive.attributes.contains("POSITION"))
				return false;

			if (!ReadVector3Accessor(Model, Primitive.attributes.at("POSITION"), OutMesh.Vertices))
				return false;

			if (Primitive.indices >= 0)
			{
				if (!ReadIndicesAccessor(Model, Primitive.indices, OutMesh.Indices))
					return false;
			}
			else
			{
				OutMesh.Indices.resize(OutMesh.Vertices.size());
				for (size_t Index = 0; Index < OutMesh.Indices.size(); Index++)
					OutMesh.Indices[Index] = (u32)Index;
			}

			if (Primitive.attributes.contains("NORMAL"))
				ReadVector3Accessor(Model, Primitive.attributes.at("NORMAL"), OutMesh.Normals);
			if (Primitive.attributes.contains("TANGENT"))
				ReadVector4Accessor(Model, Primitive.attributes.at("TANGENT"), OutMesh.Tangents);
			if (Primitive.attributes.contains("TEXCOORD_0"))
				ReadVector2Accessor(Model, Primitive.attributes.at("TEXCOORD_0"), OutMesh.UV1);
			if (Primitive.attributes.contains("TEXCOORD_1"))
				ReadVector2Accessor(Model, Primitive.attributes.at("TEXCOORD_1"), OutMesh.UV2);
			if (Primitive.attributes.contains("WEIGHTS_0"))
				ReadWeightsAccessor(Model, Primitive.attributes.at("WEIGHTS_0"), OutMesh.BoneWeights);
			if (Primitive.attributes.contains("JOINTS_0"))
				ReadIVector4Accessor(Model, Primitive.attributes.at("JOINTS_0"), OutMesh.BoneIndices);

			assert(Primitive.targets.size() == MorphCount);
			for (size_t TargetIndex = 0; TargetIndex < Primitive.targets.size(); TargetIndex++)
			{
				const std::map<std::string, int>& GltfTarget = Primitive.targets[TargetIndex];
				MeshMorphTarget& Target = OutMesh.MorphTargets.emplace_back();

				auto PositionIt = GltfTarget.find("POSITION");
				if (PositionIt != GltfTarget.end())
					ReadVector3Accessor(Model, PositionIt->second, Target.PositionDeltas);

				auto NormalIt = GltfTarget.find("NORMAL");
				if (NormalIt != GltfTarget.end())
					ReadVector3Accessor(Model, NormalIt->second, Target.NormalDeltas);

				auto TangentIt = GltfTarget.find("TANGENT");
				if (TangentIt != GltfTarget.end())
					ReadVector3Accessor(Model, TangentIt->second, Target.TangentDeltas);
			}

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
				TextureAsset.ImportSettings.Compression = CompressionMode::BC7;
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

	static void ImportLevelInternal(const char* SourcePath, const char* ImportPath, HGltfImportResult& Result)
	{
		const std::filesystem::path SourceAssetPath = std::filesystem::absolute(SourcePath);
		const HGltfImportPaths OutputPaths = BuildImportPaths(SourcePath, ImportPath);

		std::filesystem::create_directories(OutputPaths.TexturesDirectory);
		std::filesystem::create_directories(OutputPaths.MaterialsDirectory);
		std::filesystem::create_directories(OutputPaths.MeshesDirectory);
		std::filesystem::create_directories(OutputPaths.SkeletonsDirectory);
		std::filesystem::create_directories(OutputPaths.AnimationsDirectory);

		ImportProgress("Loading glTF and decoding images...");
		tinygltf::Model Model;
		std::unordered_map<int, Image> DecodedImages;
		if (!LoadGltfWithDecodedImages(SourcePath, Model, DecodedImages))
			return;

		for (const std::string& Extension : Model.extensionsUsed)
		{
			if (Extension != "KHR_lights_punctual" && Extension != "KHR_materials_emissive_strength")
				Log::Warning("[ImportLevel] Extension %s is not handled by this importer", Extension.c_str());
		}

		std::unordered_set<std::string> UsedOutputPaths;

		std::unordered_map<HGltfTextureKey, AssetRef<Texture2>, HGltfTextureKeyHash> ImportedTextures;
		std::unordered_map<int, AssetRef<Material>> ImportedMaterials;
		std::unordered_map<int, AssetRef<Skeleton>> ImportedSkeletons;
		std::unordered_map<int, Skeleton> ImportedSkeletonData;
		std::unordered_map<int, AssetRef<Mesh2>> ImportedMeshes;
		std::unordered_map<int, std::unordered_map<int, int>> SkinJointIndexByNode;
		std::unordered_map<int, int> MeshDefaultSkin;

		auto MakeRelativeAssetPath = [](const std::filesystem::path& AbsolutePath) -> std::string
		{
			return AssetSystem::Get().MakePathRelativeToBakedFolder(NormalisePathSeparators(std::filesystem::absolute(AbsolutePath).string()));
		};

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

		for (int NodeIndex = 0; NodeIndex < (int)Model.nodes.size(); NodeIndex++)
		{
			const tinygltf::Node& Node = Model.nodes[NodeIndex];
			if (Node.mesh < 0 || Node.skin < 0)
				continue;

			if (!MeshDefaultSkin.contains(Node.mesh))
			{
				MeshDefaultSkin[Node.mesh] = Node.skin;
			}
			else if (MeshDefaultSkin[Node.mesh] != Node.skin)
			{
				Log::Warning("[ImportLevel] Mesh %d is used with multiple skins; keeping first skin as default skeleton", Node.mesh);
			}
		}

		ImportProgress("Importing skeletons...");
		for (int SkinIndex = 0; SkinIndex < (int)Model.skins.size(); SkinIndex++)
		{
			const tinygltf::Skin& GltfSkin = Model.skins[SkinIndex];
			if (GltfSkin.joints.empty())
				continue;

			std::unordered_map<int, int> JointIndexByNode;
			for (int JointIndex = 0; JointIndex < (int)GltfSkin.joints.size(); JointIndex++)
			{
				JointIndexByNode[GltfSkin.joints[JointIndex]] = JointIndex;
			}

			std::vector<Matrix> InverseBindMatrices;
			if (GltfSkin.inverseBindMatrices >= 0)
				ReadMatrixAccessor(Model, GltfSkin.inverseBindMatrices, InverseBindMatrices);

			Skeleton SkeletonAsset;
			SkeletonAsset.SourcePath = NormalisePathSeparators(SourceAssetPath.string()) + "#skin:" + std::to_string(SkinIndex);
			SkeletonAsset.Joints.reserve(GltfSkin.joints.size());

			for (int JointIndex = 0; JointIndex < (int)GltfSkin.joints.size(); JointIndex++)
			{
				const int NodeIndex = GltfSkin.joints[JointIndex];
				SkeletonJoint& Joint = SkeletonAsset.Joints.emplace_back();

				if (NodeIndex < 0 || NodeIndex >= (int)Model.nodes.size())
				{
					Joint.Name = "Joint_" + std::to_string(JointIndex);
					Joint.Parent = -1;
					continue;
				}

				Joint.Name = MakeNodeName(Model, NodeIndex, "Joint");

				const int ParentNode = ParentIndices[NodeIndex];
				if (JointIndexByNode.contains(ParentNode))
				{
					Joint.Parent = JointIndexByNode[ParentNode];
					Joint.RestLocal = BuildNodeLocalTransform(Model.nodes[NodeIndex]);
				}
				else
				{
					Joint.Parent = -1;
					Joint.RestLocal = WorldTransforms[NodeIndex];
				}

				if (JointIndex < (int)InverseBindMatrices.size())
					Joint.InverseBind = InverseBindMatrices[JointIndex];
			}

			const std::string SkeletonBaseName = !GltfSkin.name.empty()
				? GltfSkin.name
				: (GltfSkin.skeleton >= 0 ? MakeNodeName(Model, GltfSkin.skeleton, "Skeleton") : ("Skeleton_" + std::to_string(SkinIndex)));
			const std::filesystem::path SkeletonPath = MakeUniqueFilePath(
				OutputPaths.SkeletonsDirectory,
				OutputPaths.LevelBaseName + "_" + SkeletonBaseName,
				".cskel",
				UsedOutputPaths);

			if (!SaveImportedBinaryAsset(SkeletonAsset, SkeletonPath))
			{
				Log::Error("[ImportLevel] Failed to save skeleton asset %s", SkeletonPath.string().c_str());
				return;
			}
			RecordImportedFile(Result, SkeletonPath);

			AssetRef<Skeleton> SkeletonRef;
			SkeletonRef.Path = MakeRelativeAssetPath(SkeletonPath);
			ReloadImportedAssetIfLoaded(SkeletonPath);
			ImportedSkeletons[SkinIndex] = SkeletonRef;
			ImportedSkeletonData[SkinIndex] = SkeletonAsset;
			SkinJointIndexByNode[SkinIndex] = std::move(JointIndexByNode);
		}

		ImportProgress("Importing animations...");
		for (int AnimIndex = 0; AnimIndex < (int)Model.animations.size(); AnimIndex++)
		{
			const tinygltf::Animation& GltfAnim = Model.animations[AnimIndex];

			int TargetSkin = -1;
			for (const tinygltf::AnimationChannel& Channel : GltfAnim.channels)
			{
				if (Channel.target_path == "weights")
					continue;

				if (Channel.target_node < 0)
					continue;

				for (int SkinIndex = 0; SkinIndex < (int)Model.skins.size(); SkinIndex++)
				{
					if (SkinJointIndexByNode.contains(SkinIndex) && SkinJointIndexByNode[SkinIndex].contains(Channel.target_node))
					{
						TargetSkin = SkinIndex;
						break;
					}
				}

				if (TargetSkin >= 0)
					break;
			}

			const Skeleton* TargetSkeleton = nullptr;
			if (TargetSkin >= 0)
			{
				if (!ImportedSkeletons.contains(TargetSkin) || !ImportedSkeletonData.contains(TargetSkin))
				{
					Log::Warning("[ImportLevel] Animation %d couldn't find target skeleton data; joint tracks will be skipped", AnimIndex);
					TargetSkin = -1;
				}
				else
				{
					TargetSkeleton = &ImportedSkeletonData[TargetSkin];
				}
			}

			AnimClip ClipAsset;
			ClipAsset.SourcePath = NormalisePathSeparators(SourceAssetPath.string()) + "#animation:" + std::to_string(AnimIndex);
			if (TargetSkin >= 0)
				ClipAsset.Skeleton = ImportedSkeletons[TargetSkin];

			std::vector<MeshMorph> MorphLayout;
			int MorphTargetNode = -1;

			for (const tinygltf::AnimationChannel& Channel : GltfAnim.channels)
			{
				if (Channel.sampler < 0 || Channel.sampler >= (int)GltfAnim.samplers.size())
					continue;
				if (Channel.target_path != "translation" && Channel.target_path != "rotation" && Channel.target_path != "scale")
				{
					if (Channel.target_path != "weights")
					{
						Log::Warning("[ImportLevel] Unsupported animation target '%s' in animation %d is skipped", Channel.target_path.c_str(), AnimIndex);
						continue;
					}
				}

				const tinygltf::AnimationSampler& Sampler = GltfAnim.samplers[Channel.sampler];
				std::vector<float> Times;
				if (!ReadFloatAccessor(Model, Sampler.input, Times) || Times.empty())
					continue;

				const EAnimInterpolation Interpolation = GetGltfAnimInterpolation(Sampler.interpolation);
				const bool bIsCubic = Interpolation == EAnimInterpolation::CubicSpline;

				if (Channel.target_path == "weights")
				{
					if (Channel.target_node < 0 || Channel.target_node >= (int)Model.nodes.size())
						continue;

					const tinygltf::Node& TargetNode = Model.nodes[Channel.target_node];
					if (TargetNode.mesh < 0 || TargetNode.mesh >= (int)Model.meshes.size())
						continue;

					const std::vector<MeshMorph> Morphs = BuildMeshMorphs(Model.meshes[TargetNode.mesh]);
					if (Morphs.empty())
						continue;

					if (MorphTargetNode >= 0 && MorphTargetNode != Channel.target_node)
					{
						Log::Warning(
							"[ImportLevel] Animation %d has morph weight tracks for both node %s and node %s. AnimClip stores one morph weight stream, so node %s is skipped.",
							AnimIndex,
							MakeNodeName(Model, MorphTargetNode, "Node").c_str(),
							MakeNodeName(Model, Channel.target_node, "Node").c_str(),
							MakeNodeName(Model, Channel.target_node, "Node").c_str());
						continue;
					}

					if (!MorphLayout.empty() && !MorphLayoutsMatch(MorphLayout, Morphs))
					{
						Log::Warning(
							"[ImportLevel] Animation %d has incompatible morph weight layouts. AnimClip stores one morph weight stream, so node %s is skipped.",
							AnimIndex,
							MakeNodeName(Model, Channel.target_node, "Node").c_str());
						continue;
					}

					if (MorphLayout.empty())
					{
						MorphLayout = Morphs;
						MorphTargetNode = Channel.target_node;
					}

					std::vector<float> RawWeights;
					if (!ReadFloatAccessor(Model, Sampler.output, RawWeights))
						continue;

					const size_t ValuesPerKey = Morphs.size();
					const size_t KeyStride = bIsCubic ? ValuesPerKey * 3 : ValuesPerKey;
					const size_t KeyValueOffset = bIsCubic ? ValuesPerKey : 0;
					ClipAsset.Duration = std::max(ClipAsset.Duration, Times.back());

					for (size_t MorphIndex = 0; MorphIndex < Morphs.size(); MorphIndex++)
					{
						AnimMorphTrack& Track = FindOrAddMorphTrack(ClipAsset, (int)MorphIndex, Morphs[MorphIndex].Name);
						Track.Interpolation = Interpolation;
						Track.Weights.clear();
						Track.Weights.reserve(Times.size());

						for (size_t KeyIndex = 0; KeyIndex < Times.size(); KeyIndex++)
						{
							const size_t ValueIndex = KeyIndex * KeyStride + KeyValueOffset + MorphIndex;
							if (ValueIndex >= RawWeights.size())
								break;

							AnimKeyFloat Key;
							Key.Time = Times[KeyIndex];
							Key.Value = RawWeights[ValueIndex];
							if (bIsCubic)
							{
								const size_t InTangentIndex = KeyIndex * KeyStride + MorphIndex;
								const size_t OutTangentIndex = KeyIndex * KeyStride + ValuesPerKey * 2 + MorphIndex;
								assert(InTangentIndex < RawWeights.size());
								assert(OutTangentIndex < RawWeights.size());
								Key.InTangent = RawWeights[InTangentIndex];
								Key.OutTangent = RawWeights[OutTangentIndex];
							}

							Track.Weights.push_back(Key);
						}
					}

					continue;
				}

				if (Channel.target_node < 0 || TargetSkin < 0 || TargetSkeleton == nullptr || !SkinJointIndexByNode[TargetSkin].contains(Channel.target_node))
					continue;

				const int JointIndex = SkinJointIndexByNode[TargetSkin][Channel.target_node];
				if (JointIndex < 0 || JointIndex >= (int)TargetSkeleton->Joints.size())
					continue;

				AnimJointTrack& Track = FindOrAddJointTrack(ClipAsset, JointIndex, TargetSkeleton->Joints[JointIndex].Name);
				ClipAsset.Duration = std::max(ClipAsset.Duration, Times.back());

				if (Channel.target_path == "translation" || Channel.target_path == "scale")
				{
					std::vector<Vector3> RawValues;
					if (!ReadVector3Accessor(Model, Sampler.output, RawValues))
						continue;

					std::vector<AnimKeyVec3>& Keys = Channel.target_path == "translation" ? Track.Translations : Track.Scales;
					EAnimInterpolation& TrackInterpolation = Channel.target_path == "translation" ? Track.TranslationInterpolation : Track.ScaleInterpolation;
					TrackInterpolation = Interpolation;
					Keys.clear();
					Keys.reserve(Times.size());

					for (size_t KeyIndex = 0; KeyIndex < Times.size(); KeyIndex++)
					{
						const size_t ValueIndex = bIsCubic ? KeyIndex * 3 + 1 : KeyIndex;
						if (ValueIndex >= RawValues.size())
							break;

						AnimKeyVec3 Key;
						Key.Time = Times[KeyIndex];
						Key.Value = RawValues[ValueIndex];
						if (bIsCubic)
						{
							const size_t InTangentIndex = KeyIndex * 3;
							const size_t OutTangentIndex = KeyIndex * 3 + 2;
							assert(InTangentIndex < RawValues.size());
							assert(OutTangentIndex < RawValues.size());
							Key.InTangent = RawValues[InTangentIndex];
							Key.OutTangent = RawValues[OutTangentIndex];
						}

						Keys.push_back(Key);
					}
				}
				else if (Channel.target_path == "rotation")
				{
					std::vector<Vector4> RawValues;
					if (!ReadVector4Accessor(Model, Sampler.output, RawValues))
						continue;

					Track.RotationInterpolation = Interpolation;
					Track.Rotations.clear();
					Track.Rotations.reserve(Times.size());

					for (size_t KeyIndex = 0; KeyIndex < Times.size(); KeyIndex++)
					{
						const size_t ValueIndex = bIsCubic ? KeyIndex * 3 + 1 : KeyIndex;
						if (ValueIndex >= RawValues.size())
							break;

						AnimKeyQuat Key;
						Key.Time = Times[KeyIndex];
						Key.Value = GltfQuaternionToEngine(RawValues[ValueIndex]);
						if (bIsCubic)
						{
							const size_t InTangentIndex = KeyIndex * 3;
							const size_t OutTangentIndex = KeyIndex * 3 + 2;
							assert(InTangentIndex < RawValues.size());
							assert(OutTangentIndex < RawValues.size());
							Key.InTangent = GltfQuaternionTangentToEngine(RawValues[InTangentIndex]);
							Key.OutTangent = GltfQuaternionTangentToEngine(RawValues[OutTangentIndex]);
						}

						Track.Rotations.push_back(Key);
					}
				}
			}

			if (ClipAsset.JointTracks.empty() && ClipAsset.MorphTracks.empty())
				continue;

			const std::string AnimBaseName = !GltfAnim.name.empty()
				? GltfAnim.name
				: ("Anim_" + std::to_string(AnimIndex));
			const std::filesystem::path AnimPath = MakeUniqueFilePath(
				OutputPaths.AnimationsDirectory,
				OutputPaths.LevelBaseName + "_" + AnimBaseName,
				".canim",
				UsedOutputPaths);

			if (!SaveImportedBinaryAsset(ClipAsset, AnimPath))
			{
				Log::Error("[ImportLevel] Failed to save animation asset %s", AnimPath.string().c_str());
				return;
			}
			RecordImportedFile(Result, AnimPath);
			ReloadImportedAssetIfLoaded(AnimPath);
		}

		const auto ImportTextureRef = [&](int TextureIndex, EGltfTextureUsage Usage) -> AssetRef<Texture2>
		{
			if (TextureIndex < 0 || TextureIndex >= (int)Model.textures.size())
			{
				Log::Error("[ImportLevel] Invalid material texture index %d", TextureIndex);
				return {};
			}

			HGltfTextureKey CacheKey{ TextureIndex, Usage };
			if (ImportedTextures.contains(CacheKey))
				return ImportedTextures[CacheKey];

			const tinygltf::Texture& GltfTexture = Model.textures[TextureIndex];
			if (GltfTexture.source < 0 || !DecodedImages.contains(GltfTexture.source))
			{
				Log::Error("[ImportLevel] Texture %d has no decoded source image", TextureIndex);
				return {};
			}

			const tinygltf::Image& GltfImage = Model.images[GltfTexture.source];
			const Image& SourceImage = DecodedImages[GltfTexture.source];
			const std::string ImageSourcePath = MakeExternalImageSourcePath(SourceAssetPath, GltfImage, GltfTexture.source);
			Log::Message("[ImportLevel] Cooking texture %s (%s)...", ImageSourcePath.c_str(), GetUsageSuffix(Usage));
			std::fflush(stdout);

			UPtr<Texture2> TextureAsset(ImportTextureAssetFromImage(SourceImage, ImageSourcePath.c_str()));
			if (!TextureAsset)
			{
				Log::Error("[ImportLevel] Failed to import texture %s", ImageSourcePath.c_str());
				return {};
			}

			ApplyGltfTextureUsageSettings(*TextureAsset, SourceImage, Usage);
			if (!RecookTextureAsset(*TextureAsset))
			{
				Log::Error("[ImportLevel] Failed to cook texture %s", ImageSourcePath.c_str());
				return {};
			}

			const std::string TextureBaseName = !GltfTexture.name.empty()
				? GltfTexture.name
				: (!GltfImage.name.empty() ? GltfImage.name : ("Texture_" + std::to_string(TextureIndex)));
			const std::filesystem::path TexturePath = MakeUniqueFilePath(
				OutputPaths.TexturesDirectory,
				TextureBaseName + "_" + GetUsageSuffix(Usage),
				".cas",
				UsedOutputPaths);

			if (!SaveImportedBinaryAsset(*TextureAsset, TexturePath))
			{
				Log::Error("[ImportLevel] Failed to save texture asset %s", TexturePath.string().c_str());
				return {};
			}
			RecordImportedFile(Result, TexturePath);

			AssetRef<Texture2> Ref;
			Ref.Path = MakeRelativeAssetPath(TexturePath);
			ImportedTextures[CacheKey] = Ref;
			return Ref;
		};

		ImportProgress("Importing materials and textures...");
		for (int MaterialIndex = 0; MaterialIndex < (int)Model.materials.size(); MaterialIndex++)
		{
			const tinygltf::Material& GltfMaterial = Model.materials[MaterialIndex];
			Material MaterialAsset;
			if (GltfMaterial.doubleSided)
				Log::Warning("[ImportLevel] Material %s requests doubleSided, which is not represented by the engine material", GltfMaterial.name.c_str());
			if (GltfMaterial.alphaMode != "OPAQUE")
				Log::Warning("[ImportLevel] Material %s alphaMode %s is not mapped; the material remains opaque", GltfMaterial.name.c_str(), GltfMaterial.alphaMode.c_str());
			if (GltfMaterial.occlusionTexture.index >= 0 && GltfMaterial.occlusionTexture.index != GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index)
				Log::Warning("[ImportLevel] Material %s has a separate occlusion texture, which is not imported", GltfMaterial.name.c_str());

			if (GltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0)
				MaterialAsset.Albedo = ImportTextureRef(GltfMaterial.pbrMetallicRoughness.baseColorTexture.index, EGltfTextureUsage::BaseColor);
			if (GltfMaterial.normalTexture.index >= 0)
				MaterialAsset.Normal = ImportTextureRef(GltfMaterial.normalTexture.index, EGltfTextureUsage::Normal);
			if (GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0)
				MaterialAsset.Orm = ImportTextureRef(GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index, EGltfTextureUsage::Orm);
			if (GltfMaterial.emissiveTexture.index >= 0)
				MaterialAsset.Emissive = ImportTextureRef(GltfMaterial.emissiveTexture.index, EGltfTextureUsage::Emissive);
			if ((GltfMaterial.pbrMetallicRoughness.baseColorTexture.index >= 0 && MaterialAsset.Albedo.Path.empty()) ||
				(GltfMaterial.normalTexture.index >= 0 && MaterialAsset.Normal.Path.empty()) ||
				(GltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0 && MaterialAsset.Orm.Path.empty()) ||
				(GltfMaterial.emissiveTexture.index >= 0 && MaterialAsset.Emissive.Path.empty()))
				return;

			double EmissiveStrength = 1.0;
			if (const auto Extension = GltfMaterial.extensions.find("KHR_materials_emissive_strength"); Extension != GltfMaterial.extensions.end())
			{
				if (Extension->second.Has("emissiveStrength"))
				{
					const auto& Value = Extension->second.Get("emissiveStrength");
					if (!Value.IsNumber() || !std::isfinite(Value.GetNumberAsDouble()) || Value.GetNumberAsDouble() < 0.0)
					{
						Log::Error("[ImportLevel] Invalid emissiveStrength in material %s", GltfMaterial.name.c_str());
						return;
					}
					EmissiveStrength = Value.GetNumberAsDouble();
				}
			}

			MaterialAsset.AlbedoFactor = Vector4(
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[0],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[1],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[2],
				(float)GltfMaterial.pbrMetallicRoughness.baseColorFactor[3]);
			MaterialAsset.EmissiveFactor = Vector4(
				(float)(GltfMaterial.emissiveFactor[0] * EmissiveStrength),
				(float)(GltfMaterial.emissiveFactor[1] * EmissiveStrength),
				(float)(GltfMaterial.emissiveFactor[2] * EmissiveStrength),
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

			if (!SaveMaterialAssetToFile(MaterialAsset, MaterialPath))
			{
				Log::Error("[ImportLevel] Failed to save material asset %s", MaterialPath.string().c_str());
				return;
			}
			RecordImportedFile(Result, MaterialPath);

			AssetRef<Material> MaterialRef;
			MaterialRef.Path = MakeRelativeAssetPath(MaterialPath);
			ImportedMaterials[MaterialIndex] = MaterialRef;
		}

		ImportProgress("Importing meshes...");
		for (int MeshIndex = 0; MeshIndex < (int)Model.meshes.size(); MeshIndex++)
		{
			const tinygltf::Mesh& GltfMesh = Model.meshes[MeshIndex];
			const std::vector<MeshMorph> Morphs = BuildMeshMorphs(GltfMesh);
			std::vector<CPUMeshResource> MeshPrimitives;
			MeshPrimitives.reserve(GltfMesh.primitives.size());

			for (const tinygltf::Primitive& Primitive : GltfMesh.primitives)
			{
				CPUMeshResource CpuMesh;
				if (ReadMeshPrimitive(Model, Primitive, Morphs.size(), CpuMesh))
					MeshPrimitives.push_back(std::move(CpuMesh));
				else
				{
					Log::Error("[ImportLevel] Failed to read a primitive of mesh %d", MeshIndex);
					return;
				}
			}

			UPtr<Mesh2> MeshAsset(CreateMeshAssetFromPrimitives(MeshPrimitives, Morphs, SourcePath));
			if (!MeshAsset || MeshAsset->Primitives.empty())
			{
				Log::Error("[ImportLevel] Mesh %d contains no imported geometry", MeshIndex);
				return;
			}

			if (MeshDefaultSkin.contains(MeshIndex))
			{
				const int SkinIndex = MeshDefaultSkin[MeshIndex];
				if (ImportedSkeletons.contains(SkinIndex))
					MeshAsset->Skeleton = ImportedSkeletons[SkinIndex];
			}

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

			if (!SaveImportedBinaryAsset(*MeshAsset, MeshPath))
			{
				Log::Error("[ImportLevel] Failed to save mesh asset %s", MeshPath.string().c_str());
				return;
			}
			RecordImportedFile(Result, MeshPath);

			AssetRef<Mesh2> MeshRef;
			MeshRef.Path = MakeRelativeAssetPath(MeshPath);
			ImportedMeshes[MeshIndex] = MeshRef;
		}

		ImportProgress("Building and saving the level...");
		HLevel LevelAsset;
		std::vector<std::unique_ptr<AThing>> OwnedThings;
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
					OwnedThings.emplace_back(LightThing);
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
				OwnedThings.emplace_back(MeshThing);
				MeshThing->Name = !Node.name.empty() ? Node.name : (!Model.meshes[Node.mesh].name.empty() ? Model.meshes[Node.mesh].name : ("Mesh_" + std::to_string(Node.mesh)));
				MeshThing->Trans = NodeTransform;
				MeshThing->Mesh = ImportedMeshes[Node.mesh];
				LevelAsset.Things.push_back(MeshThing);
			}
		}

		if (!SaveLevelAssetToFile(LevelAsset, OutputPaths.LevelPath))
		{
			Log::Error("[ImportLevel] Failed to save level asset %s", OutputPaths.LevelPath.string().c_str());
			return;
		}
		RecordImportedFile(Result, OutputPaths.LevelPath);
		Result.LevelPath = OutputPaths.LevelPath.generic_string();
	}

	HGltfImportResult ImportLevel(const char* SourcePath, const char* ImportPath)
	{
		HGltfImportResult Result;
		const size_t FirstMessage = Log::GetData().size();
		try { ImportLevelInternal(SourcePath, ImportPath, Result); }
		catch (const std::exception& Error) { Log::Error("[ImportLevel] %s", Error.what()); }
		const auto& Messages = Log::GetData();
		for (size_t Index = FirstMessage; Index < Messages.size(); ++Index)
		{
			const auto& Message = Messages[Index];
			// The log buffer includes the C string terminator in its string length.
			const std::string Text = Message.text.substr(0, Message.text.find('\0'));
			if (Message.type == Log::Type_Warning) Result.Warnings.push_back(Text);
			if (Message.type == Log::Type_Error || Message.type == Log::Type_Fatal) Result.Errors.push_back(Text);
		}
		if (Result.LevelPath.empty() && Result.Errors.empty()) Result.Errors.push_back("Import produced no level asset.");
		Result.Success = !Result.LevelPath.empty() && Result.Errors.empty();
		if (Result.Success) Log::Message("[ImportLevel] Imported %s to %s", SourcePath, Result.LevelPath.c_str());
		else Log::Error("[ImportLevel] Import failed; any files already written are partial output");
		std::fflush(stdout);
		return Result;
	}

}
