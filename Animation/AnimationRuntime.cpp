#include "AnimationRuntime.h"

#include "Math/MathUtil.h"

#include <algorithm>
#include <cassert>

namespace Columbus
{

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Pose storage

	void SkeletonPose::Resize(size_t JointCount)
	{
		Local.resize(JointCount);
		Model.resize(JointCount);
	}

	namespace Anim
	{

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Track sampling helpers

	namespace
	{
		template <typename KeyType>
		struct AnimSampleSegment
		{
			const KeyType* Prev = nullptr;
			const KeyType* Next = nullptr;
			float Alpha = 0.0f;
			float Duration = 0.0f;
		};

		template <typename T>
		static T CubicHermite(const T& P0, const T& OutTangent0, const T& P1, const T& InTangent1, float Alpha, float SegmentDuration)
		{
			const float Alpha2 = Alpha * Alpha;
			const float Alpha3 = Alpha2 * Alpha;
			const float H00 = 2.0f * Alpha3 - 3.0f * Alpha2 + 1.0f;
			const float H10 = Alpha3 - 2.0f * Alpha2 + Alpha;
			const float H01 = -2.0f * Alpha3 + 3.0f * Alpha2;
			const float H11 = Alpha3 - Alpha2;

			return
				P0 * H00 +
				OutTangent0 * (SegmentDuration * H10) +
				P1 * H01 +
				InTangent1 * (SegmentDuration * H11);
		}

		template <typename KeyType>
		static AnimSampleSegment<KeyType> FindSampleSegment(const std::vector<KeyType>& Keys, float Time)
		{
			assert(!Keys.empty());

			if (Time >= Keys.back().Time)
				return { &Keys.back(), nullptr };

			if (Time <= Keys.front().Time)
				return { &Keys.front(), nullptr };

			for (size_t Index = 1; Index < Keys.size(); Index++)
			{
				const KeyType& Next = Keys[Index];
				if (Time > Next.Time)
					continue;

				const KeyType& Prev = Keys[Index - 1];
				const float SegmentDuration = Next.Time - Prev.Time;
				if (SegmentDuration <= 0.0f)
					return { &Prev, &Next, 0.0f, 0.0f };

				const float Alpha = Math::Clamp((Time - Prev.Time) / SegmentDuration, 0.0f, 1.0f);
				return { &Prev, &Next, Alpha, SegmentDuration };
			}

			return { &Keys.back(), nullptr };
		}

		static Vector3 LerpValue(const Vector3& A, const Vector3& B, float Alpha)
		{
			return A + (B - A) * Alpha;
		}

		static Quaternion LerpValue(const Quaternion& A, const Quaternion& B, float Alpha)
		{
			return Quaternion::Slerp(A, B, Alpha);
		}

		static float LerpValue(float A, float B, float Alpha)
		{
			return A + (B - A) * Alpha;
		}

		static Vector3 CubicValue(const AnimKeyVec3& Prev, const AnimKeyVec3& Next, float Alpha, float SegmentDuration)
		{
			return CubicHermite(Prev.Value, Prev.OutTangent, Next.Value, Next.InTangent, Alpha, SegmentDuration);
		}

		static Quaternion CubicValue(const AnimKeyQuat& Prev, const AnimKeyQuat& Next, float Alpha, float SegmentDuration)
		{
			const Quaternion Result = CubicHermite(Prev.Value, Prev.OutTangent, Next.Value, Next.InTangent, Alpha, SegmentDuration);
			assert(Result.LengthSqr() > 0.000001f);
			return Result.Normalized();
		}

		static float CubicValue(const AnimKeyFloat& Prev, const AnimKeyFloat& Next, float Alpha, float SegmentDuration)
		{
			return CubicHermite(Prev.Value, Prev.OutTangent, Next.Value, Next.InTangent, Alpha, SegmentDuration);
		}

		template <typename KeyType, typename ValueType>
		static ValueType SampleTrack(const std::vector<KeyType>& Keys, EAnimInterpolation Interpolation, float Time, const ValueType& Fallback)
		{
			if (Keys.empty())
				return Fallback;

			const AnimSampleSegment<KeyType> Segment = FindSampleSegment(Keys, Time);
			if (Segment.Next == nullptr)
				return Segment.Prev->Value;

			if (Interpolation == EAnimInterpolation::Step || Segment.Duration <= 0.0f)
				return Segment.Prev->Value;

			if (Interpolation == EAnimInterpolation::CubicSpline)
				return CubicValue(*Segment.Prev, *Segment.Next, Segment.Alpha, Segment.Duration);

			return LerpValue(Segment.Prev->Value, Segment.Next->Value, Segment.Alpha);
		}

		static Transform BlendTransform(const Transform& A, const Transform& B, float Alpha)
		{
			Transform Result;
			Result.Position = LerpValue(A.Position, B.Position, Alpha);
			Result.Rotation = LerpValue(A.Rotation, B.Rotation, Alpha);
			Result.Scale = LerpValue(A.Scale, B.Scale, Alpha);
			Result.Update();
			return Result;
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		// Model-space pose helpers

		static void BuildJointModelRecursive(const Skeleton& SkeletonAsset, SkeletonPose& Pose, int JointIndex, std::vector<bool>& Visited)
		{
			if (JointIndex < 0 || JointIndex >= (int)SkeletonAsset.Joints.size())
				return;

			if (Visited[JointIndex])
				return;

			const int Parent = SkeletonAsset.Joints[JointIndex].Parent;
			Pose.Local[JointIndex].Update();
			const Matrix LocalMatrix = Pose.Local[JointIndex].GetMatrix();
			if (Parent >= 0 && Parent < (int)SkeletonAsset.Joints.size())
			{
				BuildJointModelRecursive(SkeletonAsset, Pose, Parent, Visited);
				Pose.Model[JointIndex] = Pose.Model[Parent] * LocalMatrix;
			}
			else
			{
				Pose.Model[JointIndex] = LocalMatrix;
			}

			Visited[JointIndex] = true;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Pose construction

	void BuildRestPose(const Skeleton& SkeletonAsset, SkeletonPose& OutPose)
	{
		OutPose.Resize(SkeletonAsset.Joints.size());

		for (size_t JointIndex = 0; JointIndex < SkeletonAsset.Joints.size(); JointIndex++)
		{
			OutPose.Local[JointIndex] = SkeletonAsset.Joints[JointIndex].RestLocal;
			OutPose.Local[JointIndex].Update();
		}

		BuildModelPose(SkeletonAsset, OutPose);
	}

	void BuildModelPose(const Skeleton& SkeletonAsset, SkeletonPose& InOutPose)
	{
		assert(InOutPose.Local.size() == SkeletonAsset.Joints.size());
		InOutPose.Model.resize(SkeletonAsset.Joints.size());

		std::vector<bool> Visited(SkeletonAsset.Joints.size(), false);
		for (int JointIndex = 0; JointIndex < (int)SkeletonAsset.Joints.size(); JointIndex++)
		{
			BuildJointModelRecursive(SkeletonAsset, InOutPose, JointIndex, Visited);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Clip sampling

	void SampleAnimClip(const Skeleton& SkeletonAsset, const AnimClip* Clip, float Time, SkeletonPose& OutPose)
	{
		BuildRestPose(SkeletonAsset, OutPose);

		for (const AnimJointTrack& Track : Clip->JointTracks)
		{
			int JointIndex = Track.JointIndex;
			if (JointIndex >= 0 && JointIndex < (int)SkeletonAsset.Joints.size() && !Track.JointName.empty() && SkeletonAsset.Joints[JointIndex].Name != Track.JointName)
				JointIndex = SkeletonAsset.FindJointIndexByName(Track.JointName);
			else if (JointIndex < 0 || JointIndex >= (int)SkeletonAsset.Joints.size())
				JointIndex = SkeletonAsset.FindJointIndexByName(Track.JointName);

			assert(JointIndex >= 0 && JointIndex < (int)OutPose.Local.size());

			Transform& Pose = OutPose.Local[JointIndex];
			Pose.Position = SampleTrack(Track.Translations, Track.TranslationInterpolation, Time, Pose.Position);
			Pose.Rotation = SampleTrack(Track.Rotations, Track.RotationInterpolation, Time, Pose.Rotation);
			Pose.Scale = SampleTrack(Track.Scales, Track.ScaleInterpolation, Time, Pose.Scale);
			Pose.Update();
		}

		BuildModelPose(SkeletonAsset, OutPose);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Morph sampling and blending

	float SampleAnimMorphTrack(const AnimMorphTrack& Track, float Time, float Fallback)
	{
		return SampleTrack(Track.Weights, Track.Interpolation, Time, Fallback);
	}

	void SampleAnimMorphs(const AnimClip* Clip, float Time, std::span<float> OutWeights)
	{
		for (const AnimMorphTrack& Track : Clip->MorphTracks)
		{
			assert(Track.MorphIndex >= 0 && (size_t)Track.MorphIndex < OutWeights.size());
			OutWeights[Track.MorphIndex] = SampleAnimMorphTrack(Track, Time, OutWeights[Track.MorphIndex]);
		}
	}

	void BlendMorphs(std::span<const float> WeightsA, std::span<const float> WeightsB, float Alpha, std::span<float> OutWeights)
	{
		assert(WeightsA.size() == WeightsB.size());
		assert(WeightsA.size() == OutWeights.size());

		Alpha = Math::Clamp(Alpha, 0.0f, 1.0f);
		for (size_t Index = 0; Index < OutWeights.size(); Index++)
			OutWeights[Index] = WeightsA[Index] + (WeightsB[Index] - WeightsA[Index]) * Alpha;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	// Pose blending

	void BlendSkeletonPoses(const Skeleton& SkeletonAsset, const SkeletonPose& PoseA, const SkeletonPose& PoseB, float Alpha, SkeletonPose& OutPose)
	{
		Alpha = Math::Clamp(Alpha, 0.0f, 1.0f);

		assert(&OutPose != &PoseA);
		assert(&OutPose != &PoseB);
		assert(PoseA.Local.size() == SkeletonAsset.Joints.size());
		assert(PoseB.Local.size() == SkeletonAsset.Joints.size());

		OutPose.Resize(SkeletonAsset.Joints.size());
		for (size_t JointIndex = 0; JointIndex < SkeletonAsset.Joints.size(); JointIndex++)
		{
			OutPose.Local[JointIndex] = BlendTransform(PoseA.Local[JointIndex], PoseB.Local[JointIndex], Alpha);
		}

		BuildModelPose(SkeletonAsset, OutPose);
	}

	}

}
