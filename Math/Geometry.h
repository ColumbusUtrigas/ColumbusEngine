#pragma once

#include "Vector3.h"
#include "Plane.h"
#include "Box.h"

namespace Columbus::Geometry
{

	struct Ray
	{
		Vector3 Origin;
		Vector3 Direction; // must be normalised
	};

	struct Triangle
	{
		Vector3 A, B, C;

		float   Area() const;
		Vector3 Center() const;
		Vector3 Normal() const;

		// Point must be on the triangle plane
		Vector3 CalcBarycentrics(const Vector3& Point) const;
	};

	struct Sphere
	{
		Vector3 Center;
		float Radius;
	};

	struct HitPoint
	{
		bool IsHit;
		Vector3 Point; // 0 if no hit

		static HitPoint Invalid()
		{
			return HitPoint{ false, { 0, 0, 0 } };
		}
	};

	HitPoint RayPlaneIntersection(const Ray& R, const Plane& P);
	HitPoint RayTriangleIntersection(const Ray& R, const Triangle& Tri);
	HitPoint RaySphereIntersection(const Ray& R, const Sphere& S);
	HitPoint RayBoxIntersecetion(const Ray& R, const Box& B);

}
