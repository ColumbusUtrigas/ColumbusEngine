#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	struct Plane
	{
	public:
		Vector3 normal; // must be normalized
		float d;

	public:
		Plane() : normal(0,1,0), d(0) {}
		Plane(const Vector3& normal, float d) : normal(normal), d(d) {}
		Plane(float x, float y, float z, float d) : normal(x,y,z), d(d) {}

		float DistanceToPoint(const Vector3& point)
		{
			return normal.Dot(point) - d;
		}
	};

}