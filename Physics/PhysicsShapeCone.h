#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCone : public PhysicsShape
	{
	private:
		float Radius;
		float Height;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeCone, "PhysicsShapeCone", 0.5f, 1.0f)
	public:
		PhysicsShapeCone(float Radius, float Height) :
			Radius(Radius),
			Height(Height)
		{
			mShape = new btConeShape(Radius, Height);
		}
		/*
		* Returns radius of cone collision shape
		* @return float: Radius of cone shape
		*/
		float GetRadius() const
		{
			return Radius;
		}
		/*
		* Returns height of cone collision shape
		* @return float: Height of cone shape
		*/
		float GetHeight() const
		{
			return Height;
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeCone(Radius, Height);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
