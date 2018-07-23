#pragma once

#include <Physics/PhysicsShape.h>

namespace Columbus
{

	class PhysicsShapeCapsule : public PhysicsShape
	{
	private:
		float Radius;
		float Height;
	public:
		PhysicsShapeCapsule(float Radius, float Height) :
			Radius(0.0f),
			Height(0.0f)
		{
			mShape = new btCapsuleShape(Radius, Height); 
			this->Radius = Radius;
			this->Height = Height;
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
	};

}






