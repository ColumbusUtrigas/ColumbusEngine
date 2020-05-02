#pragma once

#include <Physics/PhysicsShape.h>
#include <Scene/Transform.h>

namespace Columbus
{

	class PhysicsShapeCompuond : public PhysicsShape
	{
	protected:
		std::vector<PhysicsShape*> Shapes;
	public:
		PhysicsShapeCompuond()
		{
			mShape = new btCompoundShape();
		}
		/*
		* Add child collision shape
		* @param Transform LocalTransform: Local transform of child collision shape
		* @param PhysicsShape* Shape: New child collision shape
		*/
		void AddChildShape(Transform LocalTransform, PhysicsShape* Shape)
		{
			if (Shape != nullptr)
			{
				if (Shape->IsExist())
				{
					btTransform bTransform;
					const auto& Pos = LocalTransform.Position;
					const auto& Rot = LocalTransform.Rotation;

					bTransform.setOrigin(btVector3(Pos.X, Pos.Y, Pos.Z));
					bTransform.setRotation(btQuaternion(Rot.X, Rot.Y, Rot.Z, Rot.W));

					static_cast<btCompoundShape*>(mShape)->addChildShape(bTransform, Shape->mShape);

					Shapes.push_back(Shape);
				}
			}
		}

		~PhysicsShapeCompuond() {}
	};

}










