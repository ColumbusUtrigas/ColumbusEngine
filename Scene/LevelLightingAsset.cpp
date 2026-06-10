#include "LevelLightingAsset.h"

#include "Core/Serialisation.h"
#include "System/Log.h"

namespace Columbus
{
	std::string MakeDefaultLevelLightingDataPath(AssetSystem& Assets, const std::filesystem::path& LevelPath)
	{
		std::filesystem::path AbsoluteLevelPath = LevelPath;
		if (!AbsoluteLevelPath.is_absolute())
		{
			AbsoluteLevelPath = std::filesystem::path(Assets.DataPath) / AbsoluteLevelPath;
		}

		std::filesystem::path LightingPath = AbsoluteLevelPath;
		LightingPath.replace_extension(".clight");
		return Assets.MakePathRelativeToBakedFolder(LightingPath.string());
	}

	HLevelLightingData* LoadLevelLightingDataAssetFromFile(const char* Path)
	{
		DataStream Stream = DataStream::CreateFromFile(Path, "rb");
		if (!Stream.IsValid())
		{
			Log::Error("Couldn't load level lighting data asset %s", Path);
			return nullptr;
		}

		HLevelLightingData* Data = new HLevelLightingData();
		if (!Reflection_DeserialiseStructBinary(Stream, *Data))
		{
			delete Data;
			return nullptr;
		}

		return Data;
	}

	bool SaveLevelLightingDataAssetToFile(const HLevelLightingData& Data, const char* Path)
	{
		DataStream Stream = DataStream::CreateFromFile(Path, "wb");
		if (!Stream.IsValid())
			return false;

		return Reflection_SerialiseStructBinary(Stream, const_cast<HLevelLightingData&>(Data));
	}

	AssetRef<HLevelLightingData> EnsureLevelLightingDataAsset(AssetSystem& Assets, const char* LevelPath, AssetRef<HLevelLightingData> LightingData)
	{
		if (LightingData.IsValid())
			return LightingData;

		if (!LightingData.Path.empty())
		{
			LightingData.Resolve();
			if (LightingData.IsValid())
				return LightingData;
		}

		if (LevelPath == nullptr || LevelPath[0] == '\0')
			return {};

		const std::string LightingPath = MakeDefaultLevelLightingDataPath(Assets, LevelPath);
		if (Assets.HasPath(LightingPath))
		{
			return Assets.GetRefByPath<HLevelLightingData>(LightingPath);
		}

		const std::filesystem::path AbsoluteLightingPath = std::filesystem::path(Assets.DataPath) / LightingPath;
		if (std::filesystem::exists(AbsoluteLightingPath))
		{
			AssetRef<HLevelLightingData> ExistingLightingData = AssetRef<HLevelLightingData>(LightingPath);
			ExistingLightingData.Resolve();
			return ExistingLightingData;
		}

		return Assets.RegisterAssetRef(new HLevelLightingData(), LightingPath);
	}
}
