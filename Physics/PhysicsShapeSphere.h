#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeSphere : public PhysicsShape
	{
	private:
		float Radius;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeSphere, "PhysicsShapeSphere", 0.5f)
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

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeSphere(Radius);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
