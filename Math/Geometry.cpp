#include "Geometry.h"

namespace Columbus::Geometry
{

	// *************************************
	// Triangle functions
	float Triangle::Area() const
	{
		return Vector3::Cross(B - A, C - A).Length({}) / 2;
	}

	Vector3 Triangle::Center() const
	{
		return  (A + B + C) / 3;
	}

	Vector3 Triangle::Normal() const
	{
		return Vector3::Cross(B - A, C - A).Normalized();
	}

	Vector3 Triangle::CalcBarycentrics(const Vector3& P) const
	{
		float TriArea = Area();

		Triangle SubTri1{ C, A, P };
		Triangle SubTri2{ A, B, P };
		Triangle SubTri3{ B, C, P };

		return Vector3(SubTri1.Area() / TriArea, SubTri2.Area() / TriArea, SubTri3.Area() / TriArea);
	}
	//

	HitPoint RayPlaneIntersection(const Ray& R, const Plane& P)
	{
		Vector3 Closest = P.ClosestPoint(R.Origin);
		float Distance = P.DistanceToPoint(R.Origin);
		Vector3 ClosestDir = (Closest - R.Origin).Normalized();

		float dot = ClosestDir.Dot(R.Direction);
		if (dot < 0.001f)
		{
			// ray is parallel or looking into other direction
			return HitPoint::Invalid();
		}

		return HitPoint{ .IsHit = true, .Point = R.Origin + (fabs(Distance) / dot) * R.Direction };
	}

	HitPoint RayTriangleIntersection(const Ray& R, const Triangle& Tri)
	{
		// first, ray-plane intersection
		// then build barycentrics for the triangle, and check if it's inside

		Vector3 triCenter = Tri.Center();
		Vector3 triNormal = Tri.Normal();
		float triArea     = Tri.Area();

		Plane originPlane(triNormal, 0);
		Plane triPlane(triNormal, originPlane.DistanceToPoint(triCenter)); // plane defined by a triangle

		HitPoint triPlaneIntersection = RayPlaneIntersection(R, triPlane);
		if (!triPlaneIntersection.IsHit)
		{
			return HitPoint::Invalid();
		}

		Vector3 Barycentrics = Tri.CalcBarycentrics(triPlaneIntersection.Point);
		float   BarySum = Barycentrics.X + Barycentrics.Y + Barycentrics.Z;

		// if barycentrics sum equals to 1, point is inside of the triangle
		if ((BarySum - 1.0f) < 0.001f) // if barySum == 1
		{
			return HitPoint{ .IsHit = true, .Point = triPlaneIntersection.Point };
		}

		return HitPoint::Invalid();
	}

	HitPoint RaySphereIntersection(const Ray& R, const Sphere& S)
	{
		// not implemented
		return HitPoint::Invalid();
	}

	HitPoint RayBoxIntersecetion(const Ray& R, const Box& B)
	{
		// not implemented
		return HitPoint::Invalid();
	}

}
