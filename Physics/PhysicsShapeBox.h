#pragma once

#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class PhysicsShapeBox : public PhysicsShape
	{
	public:
		PhysicsShapeBox(Vector3 Size) { mShape = new btBoxShape(btVector3(Size.x * 0.5f, Size.y * 0.5f, Size.z * 0.5f)); }

		Vector3 GetSizeWithMargin() const
		{
			if (mShape != nullptr)
			{
				btVector3 HalfExtents = static_cast<btBoxShape*>(mShape)->getHalfExtentsWithMargin();
				return Vector3(HalfExtents.getX() * 2, HalfExtents.getY() * 2, HalfExtents.getZ() * 2);
			}

			return Vector3(1, 1, 1);
		}

		Vector3 GetSizeWithoutMargin() const
		{
			if (mShape != nullptr)
			{
				btVector3 HalfExtents = static_cast<btBoxShape*>(mShape)->getHalfExtentsWithoutMargin();
				return Vector3(HalfExtents.getX() * 2, HalfExtents.getY() * 2, HalfExtents.getZ() * 2);
			}

			return Vector3(0, 0, 0);
		}
	};

}




