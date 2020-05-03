#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCapsule : public PhysicsShape
	{
	private:
		float Radius;
		float Height;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeCapsule, "PhysicsShapeCapsule", 0.5f, 2.0f)
	public:
		PhysicsShapeCapsule(float Radius, float Height) :
			Radius(Radius),
			Height(Height)
		{
			mShape = new btCapsuleShape(Radius, Height); 
		}
		/*
		* Returns radius of capsule collision shape
		* @return float: Radius of capsule shape
		*/
		float GetRadius() const
		{
			return Radius;
		}
		/*
		* Returns height of capsule collision shape
		* @return float: Height of capsule shape
		*/
		float GetHeight() const
		{
			return Height;
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeCapsule(Radius, Height);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
