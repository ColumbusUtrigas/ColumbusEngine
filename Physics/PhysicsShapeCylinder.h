#pragma once

#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class PhysicsShapeCylinder : public PhysicsShape
	{
	private:
		Vector3 Size;
		DECLARE_PROTOTYPE(PhysicsShape, PhysicsShapeCylinder, "PhysicsShapeCylinder", Vector3{ 1, 1, 1 })
	public:
		PhysicsShapeCylinder(Vector3 Size)
		{
			mShape = new btCylinderShape(btVector3(Size.X * 0.5f, Size.Y * 0.5f, Size.Z * 0.5f));
			this->Size = Size;
		}
		/*
		* Returns size of cylinder collision shape
		* @return Vector3: Size of cylinder collision shape
		*/
		Vector3 GetSize() const
		{
			return Size;
		}

		PhysicsShape* Clone() const override
		{
			auto shape = new PhysicsShapeCylinder(Size);
			shape->SetMargin(Margin);
			return shape;
		}

		void Serialize(JSON& J) const override;
		void Deserialize(JSON& J) override;
	};

}
