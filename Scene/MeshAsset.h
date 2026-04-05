#pragma once

#include "Graphics/Mesh.h"
#include <span>

namespace Columbus
{

	// Mesh asset import/cook/load helpers. Runtime GPU upload stays in World/Graphics.
	Mesh2* LoadMeshAssetFromFile(const char* Path);
	Mesh2* CreateMeshAssetFromPrimitives(std::span<const CPUMeshResource> MeshPrimitives, const char* SourcePath);
	Mesh2* ImportMeshAssetFromSource(const char* SourcePath);
	Mesh2* RebuildMeshAssetForEditor(const Mesh2& SourceAsset);
	bool SaveMeshAssetToFile(const Mesh2& Mesh, const char* Path);

}
