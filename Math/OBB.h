#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>

namespace Columbus
{

	class OBB
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		OBB() : Min(Vector3(-1)), Max(Vector3(1)) {}
		OBB(const Vector3& InMin, const Vector3& InMax) : Min(InMin), Max(InMax) {}

		OBB operator*(const Matrix& Other) const
		{
			return OBB { (Other * Vector4(Min, 1)).XYZ(), (Other * Vector4(Max, 1)).XYZ() };
		}

		~OBB() {}
	};

}











