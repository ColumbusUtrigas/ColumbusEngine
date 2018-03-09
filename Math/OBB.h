#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	/*
	* Oriented Bounding Box class
	* Min and Max fields is Bounding Box left-down-back and right-up-face corners
	* Default values of Min and Max are (-1, -1, -1) and (1, 1, 1)
	*/
	class OBB
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		OBB() : 
			Min(Vector3(-1, -1, -1)),
			Max(Vector3(1, 1, 1)) {}

		OBB(Vector3 aMin, Vector3 aMax) :
			Min(aMin),
			Max(aMax) {}

		OBB& operator=(const OBB aOther)
		{
			this->Min = aOther.Min;
			this->Max = aOther.Max;
		}

		~OBB() {}
	};

}











