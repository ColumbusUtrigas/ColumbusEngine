#include "Geometry.h"

namespace Columbus::Geometry
{

	// *************************************
	// Triangle functions
	float Triangle::Area() const
	{
		return Vector3::Cross(B - A, C - A).Length() / 2;
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

	// *************************************
	// Intersection functions

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
		assert(false);
		return HitPoint::Invalid();
	}

	HitPoint RayBoxIntersecetion(const Ray& R, const Box& B)
	{
		// not implemented
		assert(false);
		return HitPoint::Invalid();
	}

	// *************************************
	// Closest point functions

	Vector3 LineSegmentClosestPoint(const Vector3& P, const LineSegment& L)
	{
		// compute projection of P onto L transformed to zero, assume L has non-zero length
		Vector3 Dir = (L.End - L.Start).Normalized();
		Vector3 Point = P - L.Start;
		float Proj = Dir.Dot(Point);

		Proj = Math::Clamp(Proj, 0.0f, 1.0f);
		return L.Start + (L.End - L.Start) * Proj;
	}

	Vector3 PlaneClosestPoint(const Vector3& P, const Plane& Pl)
	{
		return P - Pl.normal * PointPlaneDistance(P, Pl);
	}

	Vector3 TriangleClosestPoint(const Vector3& P, const Triangle& Tri)
	{
		// not implemented
		assert(false);
		return Vector3();
	}

	Vector3 SphereClosestPoint(const Vector3& P, const Sphere& S)
	{
		Vector3 Dir = P - S.Center;

		if (Dir.Length() > 0.001f)
		{
			return Dir.Normalized() * S.Radius;
		}

		return S.Center + Vector3(1, 0, 0) * S.Radius; // fallback if P is in the center
	}

	Vector3 BoxClosestPoint(const Vector3& P, const Box& B)
	{
		return Vector3(
			Math::Clamp(P.X, B.Min.X, B.Max.X),
			Math::Clamp(P.Y, B.Min.Y, B.Max.Y),
			Math::Clamp(P.Z, B.Min.Z, B.Max.Z)
		);
	}

	// *************************************
	// Distance functions

	float PointLineSegmentDistance(const Vector3& P, const LineSegment& L)
	{
		return LineSegmentClosestPoint(P, L).Distance(P);
	}

	float PointPlaneDistance(const Vector3& P, const Plane& Pl)
	{
		// project P onto normal
		float OriginDistance = P.Dot(Pl.normal);
		// positive distance when above the normal, negative when below
		return OriginDistance - Pl.d;
	}

	float PointTriangleDistance(const Vector3& P, const Triangle& B)
	{
		// TODO: min distance to three line segments?

		// not implemented
		assert(false);
		return 0.0f;
	}

	float PointSphereDistance(const Vector3& P, const Sphere& S)
	{
		return P.Distance(S.Center) - S.Radius;
	}

	float PointBoxDistance(const Vector3& P, const Box& B)
	{
		return BoxClosestPoint(P, B).Distance(P);
	}

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Reflection type definitions

#include "Core/Reflection.h"
#include "Quaternion.h"

using namespace Columbus;

CREFLECT_STRUCT_BEGIN(Vector2, "")
	CREFLECT_STRUCT_FIELD(float, X, "")
	CREFLECT_STRUCT_FIELD(float, Y, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(Vector3, "")
	CREFLECT_STRUCT_FIELD(float, X, "")
	CREFLECT_STRUCT_FIELD(float, Y, "")
	CREFLECT_STRUCT_FIELD(float, Z, "")
CREFLECT_STRUCT_END()

CREFLECT_STRUCT_BEGIN(Vector4, "")
	CREFLECT_STRUCT_FIELD(float, X, "")
	CREFLECT_STRUCT_FIELD(float, Y, "")
	CREFLECT_STRUCT_FIELD(float, Z, "")
	CREFLECT_STRUCT_FIELD(float, W, "")
CREFLECT_STRUCT_END()


CREFLECT_STRUCT_BEGIN(Quaternion, "")
	CREFLECT_STRUCT_FIELD(float, X, "")
	CREFLECT_STRUCT_FIELD(float, Y, "")
	CREFLECT_STRUCT_FIELD(float, Z, "")
	CREFLECT_STRUCT_FIELD(float, W, "")
CREFLECT_STRUCT_END()
