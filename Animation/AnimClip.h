#pragma once

#include "Animation/Skeleton.h"
#include "Core/Asset.h"
#include "Core/Reflection.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"

#include <string>
#include <vector>

namespace Columbus
{

	enum class EAnimInterpolation
	{
		Linear,
		Step,
		CubicSpline,
	};

	struct AnimKeyVec3
	{
		float Time = 0.0f;
		Vector3 Value = Vector3(0.0f);
		Vector3 InTangent = Vector3(0.0f);
		Vector3 OutTangent = Vector3(0.0f);
	};

	struct AnimKeyQuat
	{
		float Time = 0.0f;
		Quaternion Value;
		Quaternion InTangent = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
		Quaternion OutTangent = Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
	};

	struct AnimKeyFloat
	{
		float Time = 0.0f;
		float Value = 0.0f;
		float InTangent = 0.0f;
		float OutTangent = 0.0f;
	};

	struct AnimJointTrack
	{
		std::string JointName;
		int JointIndex = -1;

		EAnimInterpolation TranslationInterpolation = EAnimInterpolation::Linear;
		EAnimInterpolation RotationInterpolation = EAnimInterpolation::Linear;
		EAnimInterpolation ScaleInterpolation = EAnimInterpolation::Linear;

		std::vector<AnimKeyVec3> Translations;
		std::vector<AnimKeyQuat> Rotations;
		std::vector<AnimKeyVec3> Scales;
	};

	struct AnimMorphTrack
	{
		std::string MorphName;
		int MorphIndex = -1;

		EAnimInterpolation Interpolation = EAnimInterpolation::Linear;
		std::vector<AnimKeyFloat> Weights;
	};

	struct AnimClip
	{
		std::string SourcePath;
		AssetRef<Skeleton> Skeleton;
		float Duration = 0.0f;
		std::vector<AnimJointTrack> JointTracks;
		std::vector<AnimMorphTrack> MorphTracks;
	};

}

CREFLECT_DECLARE_ENUM(Columbus::EAnimInterpolation, "A38FC08A-0F6E-4BE2-8A27-D492AA7B747E");
CREFLECT_DECLARE_STRUCT(Columbus::AnimKeyVec3, 2, "968F3C31-1A03-4A4A-AA1A-9B4DC30B97F7");
CREFLECT_DECLARE_STRUCT(Columbus::AnimKeyQuat, 2, "05D70D8F-0609-4FAC-B561-E1BEBE802E65");
CREFLECT_DECLARE_STRUCT(Columbus::AnimKeyFloat, 2, "4F8F6F8E-5481-4A5C-87E4-85E0320F9CE7");
CREFLECT_DECLARE_STRUCT(Columbus::AnimJointTrack, 1, "C2A9B2C6-9831-463D-8CD3-B59A57BA8AA6");
CREFLECT_DECLARE_STRUCT(Columbus::AnimMorphTrack, 1, "0E28B1F1-410A-4B3B-AE39-CBA65E6C016D");
CREFLECT_DECLARE_STRUCT(Columbus::AnimClip, 4, "9B16992C-933A-4AD2-AC42-E67E3EC3B018");
