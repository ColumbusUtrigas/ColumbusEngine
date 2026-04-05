#include "MeshAsset.h"

#include "Core/Asset.h"
#include "Core/Serialisation.h"
#include "System/Log.h"
#include "Common/Model/Model.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <filesystem>

namespace Columbus
{

	namespace
	{
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

		static std::string MakeMeshSourcePathForAsset(const char* SourcePath)
		{
			if (SourcePath == nullptr)
				return {};

			std::string Result = NormalisePathSeparators(SourcePath);
			AssetSystem& Assets = AssetSystem::Get();

			if (!Assets.SourceDataPath.empty() && Assets.IsPathInSourceFolder(Result))
				return Assets.MakePathRelativeToSourceFolder(Result);

			return Result;
		}

		static std::string ResolveMeshSourcePath(const std::string& SourcePath)
		{
			if (SourcePath.empty())
				return {};

			std::filesystem::path Path(SourcePath);
			if (Path.is_absolute())
				return NormalisePathSeparators(Path.string());

			AssetSystem& Assets = AssetSystem::Get();
			if (!Assets.SourceDataPath.empty())
				return NormalisePathSeparators((std::filesystem::path(Assets.SourceDataPath) / Path).string());

			return NormalisePathSeparators(Path.string());
		}

		static void GenerateFallbackTangents(CPUMeshResource& Mesh)
		{
			Mesh.Tangents.resize(Mesh.Vertices.size());
			for (size_t Index = 0; Index < Mesh.Vertices.size(); Index++)
			{
				Vector3 Normal = Index < Mesh.Normals.size() ? Mesh.Normals[Index] : Vector3(0.0f, 1.0f, 0.0f);
				if (Normal.Length() <= 1e-6f)
					Normal = Vector3(0.0f, 1.0f, 0.0f);

				const Vector3 Helper = fabsf(Normal.Y) > 0.999f ? Vector3(0.0f, 0.0f, 1.0f) : Vector3(0.0f, 1.0f, 0.0f);
				const Vector3 Tangent = Vector3::Cross(Normal, Helper).Normalized();
				Mesh.Tangents[Index] = Vector4(Tangent, 1.0f);
			}
		}

		static void NormaliseMeshResource(CPUMeshResource& Mesh, const MeshImportSettings& Settings)
		{
			if (Mesh.UV1.size() != Mesh.Vertices.size())
				Mesh.UV1.assign(Mesh.Vertices.size(), Vector2(0.0f, 0.0f));

			if (Mesh.Normals.size() != Mesh.Vertices.size())
				Mesh.CalculateNormals();

			if (Settings.GenerateTangents)
				Mesh.CalculateTangents();
			else
				GenerateFallbackTangents(Mesh);
		}

		static void ApplyImportScale(CPUMeshResource& Mesh, float Scale)
		{
			if (fabsf(Scale - 1.0f) <= 1e-6f)
				return;

			for (Vector3& Vertex : Mesh.Vertices)
				Vertex *= Scale;
		}

		static Box CalculateMeshBounds(const CPUMeshResource& Mesh)
		{
			Vector3 MinVertex(FLT_MAX);
			Vector3 MaxVertex(-FLT_MAX);

			for (const Vector3& Vertex : Mesh.Vertices)
			{
				MinVertex = Vector3::Min(Vertex, MinVertex);
				MaxVertex = Vector3::Max(Vertex, MaxVertex);
			}

			if (Mesh.Vertices.empty())
				return Box(Vector3(0.0f), Vector3(0.0f));

			return Box(MinVertex, MaxVertex);
		}

		static void PostProcessMeshAsset(Mesh2& Mesh)
		{
			Mesh.BoundingBox = Box(Vector3(FLT_MAX), Vector3(-FLT_MAX));

			for (MeshPrimitive& Primitive : Mesh.Primitives)
			{
				NormaliseMeshResource(Primitive.CPU, Mesh.ImportSettings);
				Primitive.BoundingBox = CalculateMeshBounds(Primitive.CPU);
				Mesh.BoundingBox.Min = Vector3::Min(Mesh.BoundingBox.Min, Primitive.BoundingBox.Min);
				Mesh.BoundingBox.Max = Vector3::Max(Mesh.BoundingBox.Max, Primitive.BoundingBox.Max);
			}

			if (Mesh.Primitives.empty())
				Mesh.BoundingBox = Box(Vector3(0.0f), Vector3(0.0f));
		}

		static Mesh2* CreateMeshAssetFromModel(const Model& MeshModel, const char* SourcePath)
		{
			Mesh2* Result = new Mesh2();
			Result->SourcePath = MakeMeshSourcePathForAsset(SourcePath);
			Result->DefaultCollisionSettings.Shape.Type = ECollisionShape::TriMesh;

			for (size_t PrimitiveIndex = 0; PrimitiveIndex < MeshModel.GetSubModelsCount(); PrimitiveIndex++)
			{
				const SubModel& SourcePrimitive = MeshModel.GetSubModel((int)PrimitiveIndex);
				MeshPrimitive& Primitive = Result->Primitives.emplace_back();
				Primitive.Name = "Primitive " + std::to_string(PrimitiveIndex);
				CPUMeshResource& CpuMesh = Primitive.CPU;

				CpuMesh.Vertices.reserve(SourcePrimitive.VerticesCount);
				CpuMesh.Normals.reserve(SourcePrimitive.VerticesCount);
				CpuMesh.UV1.reserve(SourcePrimitive.VerticesCount);
				CpuMesh.Indices.reserve(SourcePrimitive.IndicesCount);

				for (u32 VertexIndex = 0; VertexIndex < SourcePrimitive.VerticesCount; VertexIndex++)
				{
					CpuMesh.Vertices.push_back(SourcePrimitive.Positions[VertexIndex]);
					if (SourcePrimitive.Normals)
						CpuMesh.Normals.push_back(SourcePrimitive.Normals[VertexIndex]);
					if (SourcePrimitive.UVs)
						CpuMesh.UV1.push_back(SourcePrimitive.UVs[VertexIndex]);
				}

				for (u32 Index = 0; Index < SourcePrimitive.IndicesCount; Index++)
					CpuMesh.Indices.push_back((u32)SourcePrimitive.Indices[Index]);
			}

			PostProcessMeshAsset(*Result);
			return Result;
		}

		static void ApplyMeshImportSettingsToAsset(Mesh2& Mesh)
		{
			for (MeshPrimitive& Primitive : Mesh.Primitives)
				ApplyImportScale(Primitive.CPU, Mesh.ImportSettings.ImportScale);
		}

		static Mesh2* LoadMeshAssetBinaryInternal(const char* Path)
		{
			DataStream Stream = DataStream::CreateFromFile(Path, "rb");
			if (!Stream.IsValid())
			{
				Log::Error("Couldn't open mesh asset %s", Path);
				return nullptr;
			}

			Mesh2* Mesh = new Mesh2();
			if (!Reflection_DeserialiseStructBinary(Stream, *Mesh))
			{
				delete Mesh;
				Log::Error("Couldn't deserialize mesh asset %s", Path);
				return nullptr;
			}

			PostProcessMeshAsset(*Mesh);
			return Mesh;
		}
	}

	Mesh2* LoadMeshAssetFromFile(const char* Path)
	{
		const std::string Extension = ToLowerCopy(std::filesystem::path(Path).extension().string());
		if (Extension == ".cas")
			return LoadMeshAssetBinaryInternal(Path);

		return ImportMeshAssetFromSource(Path);
	}

	Mesh2* CreateMeshAssetFromPrimitives(std::span<const CPUMeshResource> MeshPrimitives, const char* SourcePath)
	{
		Mesh2* Result = new Mesh2();
		Result->SourcePath = MakeMeshSourcePathForAsset(SourcePath);
		Result->DefaultCollisionSettings.Shape.Type = ECollisionShape::TriMesh;
		Result->Primitives.reserve(MeshPrimitives.size());

		for (size_t PrimitiveIndex = 0; PrimitiveIndex < MeshPrimitives.size(); PrimitiveIndex++)
		{
			MeshPrimitive& Primitive = Result->Primitives.emplace_back();
			Primitive.Name = "Primitive " + std::to_string(PrimitiveIndex);
			Primitive.CPU = MeshPrimitives[PrimitiveIndex];
		}

		PostProcessMeshAsset(*Result);
		return Result;
	}

	Mesh2* ImportMeshAssetFromSource(const char* SourcePath)
	{
		Model MeshModel;
		if (!MeshModel.Load(SourcePath))
			return nullptr;

		return CreateMeshAssetFromModel(MeshModel, SourcePath);
	}

	Mesh2* RebuildMeshAssetForEditor(const Mesh2& SourceAsset)
	{
		const std::string ResolvedSourcePath = ResolveMeshSourcePath(SourceAsset.SourcePath);
		if (!ResolvedSourcePath.empty() && std::filesystem::exists(ResolvedSourcePath))
		{
			if (Mesh2* ReimportedMesh = ImportMeshAssetFromSource(ResolvedSourcePath.c_str()))
			{
				ReimportedMesh->ImportSettings = SourceAsset.ImportSettings;
				ReimportedMesh->DefaultCollisionSettings = SourceAsset.DefaultCollisionSettings;
				ReimportedMesh->Sockets = SourceAsset.Sockets;
				for (size_t PrimitiveIndex = 0; PrimitiveIndex < ReimportedMesh->Primitives.size(); PrimitiveIndex++)
				{
					if (PrimitiveIndex < SourceAsset.Primitives.size())
					{
						ReimportedMesh->Primitives[PrimitiveIndex].Name = SourceAsset.Primitives[PrimitiveIndex].Name;
						ReimportedMesh->Primitives[PrimitiveIndex].DefaultMaterial = SourceAsset.Primitives[PrimitiveIndex].DefaultMaterial;
					}
				}
				ApplyMeshImportSettingsToAsset(*ReimportedMesh);
				PostProcessMeshAsset(*ReimportedMesh);
				return ReimportedMesh;
			}
		}

		Mesh2* Mesh = new Mesh2();
		Mesh->SourcePath = SourceAsset.SourcePath;
		Mesh->ImportSettings = SourceAsset.ImportSettings;
		Mesh->DefaultCollisionSettings = SourceAsset.DefaultCollisionSettings;
		Mesh->Sockets = SourceAsset.Sockets;
		Mesh->Primitives.reserve(SourceAsset.Primitives.size());

		for (const MeshPrimitive& SourcePrimitive : SourceAsset.Primitives)
		{
			MeshPrimitive& Primitive = Mesh->Primitives.emplace_back();
			Primitive.Name = SourcePrimitive.Name;
			Primitive.DefaultMaterial = SourcePrimitive.DefaultMaterial;
			Primitive.CPU = SourcePrimitive.CPU;
		}

		PostProcessMeshAsset(*Mesh);
		return Mesh;
	}

	bool SaveMeshAssetToFile(const Mesh2& Mesh, const char* Path)
	{
		DataStream Stream = DataStream::CreateFromFile(Path, "wb");
		if (!Stream.IsValid())
			return false;

		return Reflection_SerialiseStructBinary(Stream, const_cast<Mesh2&>(Mesh));
	}

}
