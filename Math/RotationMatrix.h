#pragma once

#include <Math/Matrix4.h>

namespace Columbus
{

	class RotationMatrix : public Matrix4
	{
	public:
		inline RotationMatrix(Vector3 Axis, float Angle) :
			Matrix4(1.0f)
		{
			float c = cosf(Radians(Angle));
			float s = sinf(Radians(Angle));
			float c1 = 1.0f - c;
			float m0 = mat[0], m4 = mat[4], m8 = mat[8], m12 = mat[12],
			      m1 = mat[1], m5 = mat[5], m9 = mat[9], m13 = mat[13],
			      m2 = mat[2], m6 = mat[6], m10 = mat[10], m14 = mat[14];

			float x = Axis.x,
			      y = Axis.y,
			      z = Axis.z;

			float r0 = x * x * c1 + c;
			float r1 = x * y * c1 + z * s;
			float r2 = x * z * c1 - y * s;
			float r4 = x * y * c1 - z * s;
			float r5 = y * y * c1 + c;
			float r6 = y * z * c1 + x * s;
			float r8 = x * z * c1 + y * s;
			float r9 = y * z * c1 - x * s;
			float r10 = z * z * c1 + c;

			mat[0] = r0 * m0 + r4 * m1 + r8 * m2;
			mat[1] = r1 * m0 + r5 * m1 + r9 * m2;
			mat[2] = r2 * m0 + r6 * m1 + r10* m2;
			mat[4] = r0 * m4 + r4 * m5 + r8 * m6;
			mat[5] = r1 * m4 + r5 * m5 + r9 * m6;
			mat[6] = r2 * m4 + r6 * m5 + r10* m6;
			mat[8] = r0 * m8 + r4 * m9 + r8 * m10;
			mat[9] = r1 * m8 + r5 * m9 + r9 * m10;
			mat[10] = r2 * m8 + r6 * m9 + r10* m10;
			mat[12] = r0 * m12 + r4 * m13 + r8 * m14;
			mat[13] = r1 * m12 + r5 * m13 + r9 * m14;
			mat[14] = r2 * m12 + r6 * m13 + r10* m14;
		}

		inline static Matrix4 Make(Vector3 Axis, float Angle)
		{
			return RotationMatrix(Axis, Angle);
		}
	};

}







