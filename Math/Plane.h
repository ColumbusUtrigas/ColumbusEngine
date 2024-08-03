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

		float DistanceToPoint(const Vector3& point) const
		{
			// projection of point onto normal line and getting distance
			return normal.Dot(point) - d;
		}

		Vector3 ClosestPoint(const Vector3& point) const
		{
			// distance of point to plane multiplied by normal is an offset for point
			return point - (normal.Dot(point) - d) * normal;
		}
	};

}
