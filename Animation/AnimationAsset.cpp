#include "AnimationAsset.h"

#include "Animation/AnimClip.h"
#include "Animation/Skeleton.h"
#include "Core/Asset.h"
#include "Core/Serialisation.h"
#include "System/Log.h"

namespace Columbus
{

	namespace
	{
		template <typename T>
		static T* LoadBinaryAsset(const char* Path, const char* Label)
		{
			DataStream Stream = DataStream::CreateFromFile(Path, "rb");
			if (!Stream.IsValid())
			{
				Log::Error("Couldn't open %s asset %s", Label, Path);
				return nullptr;
			}

			T* Asset = new T();
			if (!Reflection_DeserialiseStructBinary(Stream, *Asset))
			{
				delete Asset;
				Log::Error("Couldn't deserialize %s asset %s", Label, Path);
				return nullptr;
			}

			return Asset;
		}

		template <typename T>
		static bool SaveBinaryAsset(const T& Asset, const char* Path)
		{
			DataStream Stream = DataStream::CreateFromFile(Path, "wb");
			if (!Stream.IsValid())
				return false;

			return Reflection_SerialiseStructBinary(Stream, const_cast<T&>(Asset));
		}
	}

	Skeleton* LoadSkeletonAssetFromFile(const char* Path)
	{
		return LoadBinaryAsset<Skeleton>(Path, "skeleton");
	}

	bool SaveSkeletonAssetToFile(const Skeleton& Asset, const char* Path)
	{
		return SaveBinaryAsset(Asset, Path);
	}

	AnimClip* LoadAnimClipAssetFromFile(const char* Path)
	{
		return LoadBinaryAsset<AnimClip>(Path, "animation clip");
	}

	bool SaveAnimClipAssetToFile(const AnimClip& Asset, const char* Path)
	{
		return SaveBinaryAsset(Asset, Path);
	}

	void RegisterAnimationAssetLoaders(AssetSystem& Assets)
	{
		Reflection::EnforceTypeLinkage<Skeleton>();
		Reflection::EnforceTypeLinkage<SkeletonJoint>();
		Reflection::EnforceTypeLinkage<Matrix>();
		Reflection::EnforceTypeLinkage<AnimClip>();
		Reflection::EnforceTypeLinkage<AnimJointTrack>();
		Reflection::EnforceTypeLinkage<AnimMorphTrack>();
		Reflection::EnforceTypeLinkage<AnimKeyVec3>();
		Reflection::EnforceTypeLinkage<AnimKeyQuat>();
		Reflection::EnforceTypeLinkage<AnimKeyFloat>();
		Reflection::EnforceTypeLinkage<EAnimInterpolation>();

		Assets.AssetLoaderFunctions[Reflection::FindStruct<Skeleton>()] = [](const char* Path) -> void*
		{
			return LoadSkeletonAssetFromFile(Path);
		};
		Assets.AssetUnloaderFunctions[Reflection::FindStruct<Skeleton>()] = [](void* Asset)
		{
			delete static_cast<Skeleton*>(Asset);
		};
		Assets.AssetExtensions[Reflection::FindStruct<Skeleton>()] = "cskel";

		Assets.AssetLoaderFunctions[Reflection::FindStruct<AnimClip>()] = [](const char* Path) -> void*
		{
			return LoadAnimClipAssetFromFile(Path);
		};
		Assets.AssetUnloaderFunctions[Reflection::FindStruct<AnimClip>()] = [](void* Asset)
		{
			delete static_cast<AnimClip*>(Asset);
		};
		Assets.AssetExtensions[Reflection::FindStruct<AnimClip>()] = "canim";
	}

}
