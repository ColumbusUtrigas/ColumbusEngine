#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#include <cmath>

namespace Columbus
{

	class Quaternion
	{
	public:
		float X;
		float Y;
		float Z;
		float W;
	public:
		Quaternion() :
			X(0.0f),
			Y(0.0f),
			Z(0.0f),
			W(0.0f)
		{}

		Quaternion(float X, float Y, float Z, float W) :
			X(X),
			Y(Y),
			Z(Z),
			W(W)
		{}

		Quaternion(const Vector3& Euler)
		{
			Vector3 E = Euler * 0.5f;
			Vector3 C(cosf(E.X), cosf(E.Y), cosf(E.Z));
			Vector3 S(sinf(E.X), sinf(E.Y), sinf(E.Z));

			X = S.X * C.Y * C.Z - C.X * S.Y * S.Z;
			Y = C.X * S.Y * C.Z + S.X * C.Y * S.Z;
			Z = C.X * C.Y * S.Z - S.X * S.Y * C.Z;
			W = C.X * C.Y * C.Z + S.X * S.Y * S.Z;
		}

		Quaternion(const Vector3& Axis, float Angle)
		{
			float Sin = sinf(Angle * 0.5f);
			float Cos = cosf(Angle * 0.5f);

			X = Axis.X * Sin;
			Y = Axis.Y * Sin;
			Z = Axis.Z * Sin;
			W = Cos;

			Normalize();
		}

		Vector3 XYZ() const { return Vector3(X, Y, Z); }

		Quaternion operator+() const
		{
			return *this;
		}

		// Negative quaternion, this also named "quaternion conjugation"
		Quaternion operator-() const
		{
			return Quaternion(-X, -Y, -Z, W);
		}

		Quaternion operator*(const Quaternion& Other) const
		{
			Quaternion Result;
			Quaternion P(*this);
			Quaternion Q(Other);

			Result.X = P.W * Q.X + P.X * Q.W + P.Y * Q.Z - P.Z * Q.Y;
			Result.Y = P.W * Q.Y + P.Y * Q.W + P.Z * Q.X - P.X * Q.Z;
			Result.Z = P.W * Q.Z + P.Z * Q.W + P.X * Q.Y - P.Y * Q.X;
			Result.W = P.W * Q.W - P.X * Q.X - P.Y * Q.Y - P.Z * Q.Z;

			return Result;
		}

		Quaternion operator*(float Scalar) const
		{
			return Quaternion(X * Scalar, Y * Scalar, Z * Scalar, W * Scalar);
		}

		friend Quaternion operator*(float Scalar, const Quaternion& Other)
		{
			return Other * Scalar;
		}

		Quaternion& operator*=(const Quaternion& Other)
		{
			return *this = *this * Other;
		}

		Quaternion& operator*=(float Other)
		{
			return *this = *this * Other;
		}

		float Length() const
		{
			return sqrtf(X*X + Y*Y + Z*Z + W*W);
		}

		Quaternion Normalized() const
		{
			float Inv = 1.0f / Length();
			return *this * Inv;
		}

		Quaternion& Normalize()
		{
			return *this = Normalized();
		}

		Matrix ToMatrix() const
		{
			Matrix Result(1.0f);

			float Qxx = X * X;
			float Qyy = Y * Y;
			float Qzz = Z * Z;
			float Qxz = X * Z;
			float Qxy = X * Y;
			float Qyz = Y * Z;
			float Qwx = W * X;
			float Qwy = W * Y;
			float Qwz = W * Z;

			Result.M[0][0] = 1.0f - 2.0f * (Qyy +  Qzz);
			Result.M[0][1] = 2.0f * (Qxy + Qwz);
			Result.M[0][2] = 2.0f * (Qxz - Qwy);

			Result.M[1][0] = 2.0f * (Qxy - Qwz);
			Result.M[1][1] = 1.0f - 2.0f * (Qxx +  Qzz);
			Result.M[1][2] = 2.0f * (Qyz + Qwx);

			Result.M[2][0] = 2.0f * (Qxz + Qwy);
			Result.M[2][1] = 2.0f * (Qyz - Qwx);
			Result.M[2][2] = 1.0f - 2.0f * (Qxx +  Qyy);

			return Result;
		}

		/*
		* Convert Quaternion into Euler angle (in degrees)
		*/
		Vector3 Euler() const;

		~Quaternion() {}
	};

}


