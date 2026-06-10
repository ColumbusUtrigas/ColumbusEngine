#pragma once

#include "Core/Asset.h"
#include "Graphics/IrradianceVolume.h"

#include <filesystem>
#include <string>

namespace Columbus
{
	std::string MakeDefaultLevelLightingDataPath(AssetSystem& Assets, const std::filesystem::path& LevelPath);
	HLevelLightingData* LoadLevelLightingDataAssetFromFile(const char* Path);
	bool SaveLevelLightingDataAssetToFile(const HLevelLightingData& Data, const char* Path);
	AssetRef<HLevelLightingData> EnsureLevelLightingDataAsset(AssetSystem& Assets, const char* LevelPath, AssetRef<HLevelLightingData> LightingData);
}
