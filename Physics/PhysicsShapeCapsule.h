#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCapsule : public PhysicsShape
	{
	public:
		PhysicsShapeCapsule(float Radius, float Height) { mShape = new btCapsuleShape(Radius, Height);  }
		/*
		* Returns radius of capsule shape
		*/
		float GetRadius() const
		{
			if (mShape != nullptr)
			{
				return static_cast<btCapsuleShape*>(mShape)->getRadius();
			}

			return 0.0f;
		}
		/*
		* Returns height of capsule shape
		*/
		float GetHeight() const
		{
			if (mShape != nullptr)
			{
				return static_cast<btCapsuleShape*>(mShape)->getHalfHeight() * 2;
			}

			return 0.0f;
		}
	};

}






