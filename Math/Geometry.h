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

	struct LineSegment
	{
		Vector3 Start;
		Vector3 End;
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

	Vector3 LineSegmentClosestPoint(const Vector3& P, const LineSegment& L);
	Vector3 PlaneClosestPoint(const Vector3& P, const Plane& Pl);
	Vector3 TriangleClosestPoint(const Vector3& P, const Triangle& Tri);
	Vector3 SphereClosestPoint(const Vector3& P, const Sphere& S);
	Vector3 BoxClosestPoint(const Vector3& P, const Box& B);

	float PointLineSegmentDistance(const Vector3& P, const LineSegment& L);
	float PointPlaneDistance(const Vector3& P, const Plane& Pl);
	float PointTriangleDistance(const Vector3& P, const Triangle& Tri);
	float PointSphereDistance(const Vector3& P, const Sphere& S);
	float PointBoxDistance(const Vector3& P, const Box& B);

}
