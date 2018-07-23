#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCone : public PhysicsShape
	{
	private:
		float Radius;
		float Height;
	public:
		PhysicsShapeCone(float Radius, float Height) :
			Radius(0.0f),
			Height(0.0f)
		{
			mShape = new btConeShape(Radius, Height);
			this->Radius = Radius;
			this->Height = Height;
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
	};

}




