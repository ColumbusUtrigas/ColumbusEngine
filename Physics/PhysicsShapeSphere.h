#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeSphere : public PhysicsShape
	{
	private:
		float Radius;
	public:
		PhysicsShapeSphere(float Radius)
		{
			if (Radius >= 0.0f)
			{
				mShape = new btSphereShape(Radius);
				this->Radius = Radius;
			}
		}
		/*
		* Returns radius of sphere collision shape
		* @return float: Radius of sphere shape
		*/
		float GetRadius() const
		{
			return Radius;
		}
	};

}




