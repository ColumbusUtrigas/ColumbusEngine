#include "AnimClip.h"

using namespace Columbus;

CREFLECT_ENUM_BEGIN(EAnimInterpolation, "")
	CREFLECT_ENUM_FIELD(EAnimInterpolation::Linear, 0)
	CREFLECT_ENUM_FIELD(EAnimInterpolation::Step, 1)
	CREFLECT_ENUM_FIELD(EAnimInterpolation::CubicSpline, 2)
CREFLECT_ENUM_END()

CREFLECT_STRUCT_BEGIN(AnimKeyVec3, "")
	CREFLECT_STRUCT_FIELD(float, Time, "")
	CREFLECT_STRUCT_FIELD(Vector3, Value, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Vector3, InTangent, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Vector3, OutTangent, "ColourChannels")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(AnimKeyQuat, "")
	CREFLECT_STRUCT_FIELD(float, Time, "")
	CREFLECT_STRUCT_FIELD(Quaternion, Value, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Quaternion, InTangent, "ColourChannels")
	CREFLECT_STRUCT_FIELD(Quaternion, OutTangent, "ColourChannels")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(AnimKeyFloat, "")
	CREFLECT_STRUCT_FIELD(float, Time, "")
	CREFLECT_STRUCT_FIELD(float, Value, "")
	CREFLECT_STRUCT_FIELD(float, InTangent, "")
	CREFLECT_STRUCT_FIELD(float, OutTangent, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(AnimJointTrack, "")
	CREFLECT_STRUCT_FIELD(std::string, JointName, "")
	CREFLECT_STRUCT_FIELD(int, JointIndex, "Noedit")
	CREFLECT_STRUCT_FIELD(EAnimInterpolation, TranslationInterpolation, "")
	CREFLECT_STRUCT_FIELD(EAnimInterpolation, RotationInterpolation, "")
	CREFLECT_STRUCT_FIELD(EAnimInterpolation, ScaleInterpolation, "")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimKeyVec3, Translations, "")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimKeyQuat, Rotations, "")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimKeyVec3, Scales, "")
CREFLECT_STRUCT_END()

static std::string GetAnimJointTrackArrayLabel(const void* Object, int Index)
{
	const AnimJointTrack* Track = reinterpret_cast<const AnimJointTrack*>(Object);
	if (Track && !Track->JointName.empty())
		return Track->JointName;
	return "Track " + std::to_string(Index);
}
CREFLECT_STRUCT_ARRAY_LABEL(AnimJointTrack, GetAnimJointTrackArrayLabel)

CREFLECT_STRUCT_BEGIN(AnimMorphTrack, "")
	CREFLECT_STRUCT_FIELD(std::string, MorphName, "")
	CREFLECT_STRUCT_FIELD(int, MorphIndex, "Noedit")
	CREFLECT_STRUCT_FIELD(EAnimInterpolation, Interpolation, "")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimKeyFloat, Weights, "")
CREFLECT_STRUCT_END()

static std::string GetAnimMorphTrackArrayLabel(const void* Object, int Index)
{
	const AnimMorphTrack* Track = reinterpret_cast<const AnimMorphTrack*>(Object);
	if (Track && !Track->MorphName.empty())
		return Track->MorphName;
	return "Morph " + std::to_string(Index);
}
CREFLECT_STRUCT_ARRAY_LABEL(AnimMorphTrack, GetAnimMorphTrackArrayLabel)

CREFLECT_STRUCT_BEGIN(AnimClip, "")
	CREFLECT_STRUCT_FIELD(std::string, SourcePath, "Noedit")
	CREFLECT_STRUCT_FIELD_ASSETREF(Skeleton, Skeleton, "")
	CREFLECT_STRUCT_FIELD(float, Duration, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimJointTrack, JointTracks, "")
	CREFLECT_STRUCT_FIELD_ARRAY(AnimMorphTrack, MorphTracks, "")
CREFLECT_STRUCT_END()
