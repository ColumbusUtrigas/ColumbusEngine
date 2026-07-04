#include "Skeleton.h"

namespace Columbus
{

	int Skeleton::FindJointIndexByName(const std::string& Name) const
	{
		for (int Index = 0; Index < (int)Joints.size(); Index++)
		{
			if (Joints[Index].Name == Name)
				return Index;
		}

		return -1;
	}

}

using namespace Columbus;

CREFLECT_STRUCT_BEGIN(SkeletonJoint, "")
	CREFLECT_STRUCT_FIELD(std::string, Name, "")
	CREFLECT_STRUCT_FIELD(int, Parent, "Noedit")
	CREFLECT_STRUCT_FIELD(Transform, RestLocal, "")
	CREFLECT_STRUCT_FIELD(Matrix, InverseBind, "Noedit")
CREFLECT_STRUCT_END()

static std::string GetSkeletonJointArrayLabel(const void* Object, int Index)
{
	const SkeletonJoint* Joint = reinterpret_cast<const SkeletonJoint*>(Object);
	if (Joint && !Joint->Name.empty())
		return Joint->Name;
	return "Joint " + std::to_string(Index);
}
CREFLECT_STRUCT_ARRAY_LABEL(SkeletonJoint, GetSkeletonJointArrayLabel)

CREFLECT_STRUCT_BEGIN(Skeleton, "")
	CREFLECT_STRUCT_FIELD(std::string, SourcePath, "Noedit")
	CREFLECT_STRUCT_FIELD_ARRAY(SkeletonJoint, Joints, "")
CREFLECT_STRUCT_END()
