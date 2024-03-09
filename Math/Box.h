#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#include <utility>

namespace Columbus
{

	// axis-aligned bounding box
	class Box
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		Box() : Min(0), Max(0) {}
		Box(const Vector3& InMin, const Vector3& InMax) : Min(InMin), Max(InMax) {}

		Vector3 CalcCenter() const { return (Min + Max) / 2; }
		Vector3 CalcSize() const { return (Max - Min); }

		// calculates AABB of a transformed box
		Box CalcTransformedBox(const Matrix& Transformation) const;

		Box operator+(const Vector3& Other) const
		{
			return Box(Min + Other, Max + Other);
		}

		Box operator*(const Vector3& Other) const
		{
			return Box(Min * Other, Max * Other);
		}

		Box operator*(const Matrix& Other) const
		{
			return Box((Vector4(Min, 1) * Other).XYZ(), (Vector4(Max, 1) * Other).XYZ());
		}

		// TODO: move to Geometry?
		bool Intersects(const Vector3& Origin, const Vector3& Direction)
		{
			float tmin = (Min.X - Origin.X) / Direction.X;
			float tmax = (Max.X - Origin.X) / Direction.X;

			if (tmin > tmax) std::swap(tmin, tmax); 

			float tymin = (Min.Y - Origin.Y) / Direction.Y; 
			float tymax = (Max.Y - Origin.Y) / Direction.Y; 

			if (tymin > tymax) std::swap(tymin, tymax); 

			if ((tmin > tymax) || (tymin > tmax)) 
				return false; 

			if (tymin > tmin) 
				tmin = tymin; 

			if (tymax < tmax) 
				tmax = tymax; 

			float tzmin = (Min.Z - Origin.Z) / Direction.Z;
			float tzmax = (Max.Z - Origin.Z) / Direction.Z;

			if (tzmin > tzmax) std::swap(tzmin, tzmax); 

			if ((tmin > tzmax) || (tzmin > tmax)) 
				return false; 

			if (tzmin > tmin) 
				tmin = tzmin; 

			if (tzmax < tmax) 
				tmax = tzmax; 

			return true; 
		}

		~Box() {}
	};

}
