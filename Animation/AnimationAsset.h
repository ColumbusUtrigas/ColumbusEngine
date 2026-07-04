#pragma once

namespace Columbus
{

	struct AnimClip;
	struct AssetSystem;
	struct Skeleton;

	Skeleton* LoadSkeletonAssetFromFile(const char* Path);
	bool SaveSkeletonAssetToFile(const Skeleton& Asset, const char* Path);

	AnimClip* LoadAnimClipAssetFromFile(const char* Path);
	bool SaveAnimClipAssetToFile(const AnimClip& Asset, const char* Path);

	void RegisterAnimationAssetLoaders(AssetSystem& Assets);

}
