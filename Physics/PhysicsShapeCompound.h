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
					Vector3 Pos = LocalTransform.GetPos();
					Vector3 Rot = LocalTransform.GetRot();

					if (Rot.x > 180) Rot.x -= 360;
					if (Rot.y > 180) Rot.y -= 360;
					if (Rot.z > 180) Rot.z -= 360;

					Rot.x = Radians(Rot.x);
					Rot.y = Radians(Rot.y);
					Rot.z = Radians(Rot.z);

					btQuaternion bRot; bRot.setEulerZYX(Rot.z, Rot.y, Rot.x);

					bTransform.setOrigin(btVector3(Pos.x, Pos.y, Pos.z));
					bTransform.setRotation(bRot);

					static_cast<btCompoundShape*>(mShape)->addChildShape(bTransform, Shape->mShape);

					Shapes.push_back(Shape);
				}
			}
		}

		~PhysicsShapeCompuond() {}
	};

}










