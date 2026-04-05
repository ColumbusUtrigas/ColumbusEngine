#pragma once

#include "Graphics/Core/Texture.h"
#include "Graphics/Vulkan/DeviceVulkan.h"

namespace Columbus
{

	// Texture asset import/cook/load helpers. Runtime texture representation stays in Graphics/Core.
	Texture2* LoadTextureAssetForRuntime(SPtr<DeviceVulkan> Device, const char* Path);
	Texture2* ImportTextureAssetFromImage(const Image& SourceImage, const char* SourcePath);
	Texture2* ImportTextureAssetFromSource(SPtr<DeviceVulkan> Device, const char* SourcePath);
	Texture2* RebuildTextureAssetForEditor(SPtr<DeviceVulkan> Device, const Texture2& SourceAsset);
	bool RecookTextureAsset(Texture2& Texture);
	bool SaveTextureAssetToFile(const Texture2& Texture, const char* Path);

}
