#pragma once

#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class PhysicsShapeBox : public PhysicsShape
	{
	private:
		Vector3 Size;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeBox, "PhysicsShapeBox", Vector3{ 1, 1, 1 })
	public:
		PhysicsShapeBox(Vector3 Size)
		{
			mShape = new btBoxShape(btVector3(Size.X * 0.5f, Size.Y * 0.5f, Size.Z * 0.5f));
			this->Size = Size;
		}
		/*
		* Returns size of box collision shape
		* @return Vector3: Size of box shape
		*/
		Vector3 GetSize() const
		{
			return Size;
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeBox(Size);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
