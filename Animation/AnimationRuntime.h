#pragma once

#include "Animation/AnimClip.h"
#include "Animation/Skeleton.h"
#include "Math/Matrix.h"

#include <span>
#include <vector>

namespace Columbus
{

	struct SkeletonPose
	{
		std::vector<Transform> Local;
		std::vector<Matrix> Model;

		void Resize(size_t JointCount);
	};

	namespace Anim
	{
		void BuildRestPose(const Skeleton& SkeletonAsset, SkeletonPose& OutPose);
		void BuildModelPose(const Skeleton& SkeletonAsset, SkeletonPose& InOutPose);
		void SampleAnimClip(const Skeleton& SkeletonAsset, const AnimClip* Clip, float Time, SkeletonPose& OutPose);
		void BlendSkeletonPoses(const Skeleton& SkeletonAsset, const SkeletonPose& PoseA, const SkeletonPose& PoseB, float Alpha, SkeletonPose& OutPose);
		float SampleAnimMorphTrack(const AnimMorphTrack& Track, float Time, float Fallback);
		void SampleAnimMorphs(const AnimClip* Clip, float Time, std::span<float> OutWeights);
		void BlendMorphs(std::span<const float> WeightsA, std::span<const float> WeightsB, float Alpha, std::span<float> OutWeights);
	}

}
