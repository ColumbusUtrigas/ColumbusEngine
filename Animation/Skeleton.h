#pragma once

#include "Core/Reflection.h"
#include "Math/Matrix.h"
#include "Scene/Transform.h"

#include <string>
#include <vector>

namespace Columbus
{

	struct SkeletonJoint
	{
		std::string Name;
		int Parent = -1;
		Transform RestLocal;
		Matrix InverseBind;
	};

	struct Skeleton
	{
		std::string SourcePath;
		std::vector<SkeletonJoint> Joints;

		int FindJointIndexByName(const std::string& Name) const;
	};

}

CREFLECT_DECLARE_STRUCT(Columbus::SkeletonJoint, 1, "B66D29A6-8378-47D7-A7D1-621ADCCD60D0");
CREFLECT_DECLARE_STRUCT(Columbus::Skeleton, 1, "C7CC5551-F917-4762-B5A0-8252C595EA07");
