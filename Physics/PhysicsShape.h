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
		/*
		* Returns true if Shape exits, else returns false
		* @return bool: Exits-value
		*/
		virtual bool IsExist() const
		{
			return mShape != nullptr;
		}
		/*
		* Sets collision margin into collision shape
		* @param float Margin: New collision margin of shape
		*/
		virtual void SetMargin(float Margin)
		{
			if (mShape != nullptr)
			{
				mShape->setMargin(Margin);
				this->Margin = Margin;
			}
		}
		/*
		* Returns collision margin of collision shape
		* @return float: Collision margin of shape
		*/
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




