#pragma once

#include <btBulletDynamicsCommon.h>

namespace Columbus
{

	class PhysicsShape
	{
	protected:
		float Margin;
	public:
		btCollisionShape* mShape;
	public:
		PhysicsShape() : mShape(nullptr), Margin(0.0f) {}

		virtual void SetMargin(float Margin)
		{
			if (mShape != nullptr)
			{
				mShape->setMargin(Margin);
				this->Margin = Margin;
			}
		}

		virtual float GetMargin() const
		{
			if (mShape != nullptr)
			{
				return Margin;
			}

			return 0.0f;
		}

		~PhysicsShape() { delete mShape; }
	};

}




