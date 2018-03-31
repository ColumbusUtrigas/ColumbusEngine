#pragma once

#include <Physics/PhysicsShape.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class PhysicsShapeBox : public PhysicsShape
	{
	private:
		Vector3 Size;
	public:
		PhysicsShapeBox(Vector3 Size)
		{
			mShape = new btBoxShape(btVector3(Size.x * 0.5f, Size.y * 0.5f, Size.z * 0.5f));
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
	};

}




