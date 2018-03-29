#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCone : public PhysicsShape
	{
	public:
		PhysicsShapeCone(float Radius, float Height) { mShape = new btConeShape(Radius, Height); }
		/*
		* Returns radius of cone shape
		*/
		float GetRadius() const
		{
			if (mShape != nullptr)
			{
				return static_cast<btConeShape*>(mShape)->getRadius();
			}

			return 0.0f;
		}
		/*
		* Returns height of cone shape
		*/
		float GetHeight() const
		{
			if (mShape != nullptr)
			{
				return static_cast<btConeShape*>(mShape)->getHeight();
			}

			return 0.0f;
		}
	};

}




