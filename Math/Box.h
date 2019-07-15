#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>

namespace Columbus
{

	class Box
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		Box() : Min(0), Max(0) {}
		Box(const Vector3& InMin, const Vector3& InMax) : Min(InMin), Max(InMax) {}

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

		~Box() {}
	};

}


