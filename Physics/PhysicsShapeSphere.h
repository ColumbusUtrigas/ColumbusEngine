#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeSphere : public PhysicsShape
	{
	public:
		PhysicsShapeSphere(float Radius) { mShape = new btSphereShape(Radius); }
	};

}




