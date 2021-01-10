#pragma once

#include <Math/Vector3.h>
#include <Math/Matrix.h>
#define _USE_MATH_DEFINES
#include <math.h>

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
			W(1.0f)
		{}

		Quaternion(float X, float Y, float Z, float W) :
			X(X),
			Y(Y),
			Z(Z),
			W(W)
		{}

		/*
		* Convert Euler angles (in degrees) to quaternion
		*/
		Quaternion(const Vector3& Euler)
		{
			Vector3 E = Euler;
			E.X = Math::Radians(E.X);
			E.Y = Math::Radians(E.Y);
			E.Z = Math::Radians(E.Z);
			E *= 0.5f;
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

		/*
		* Negative quaternion, this also named "quaternion conjugation"
		*/
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

		Quaternion operator/(const Quaternion& Other) const
		{
			return (*this) * Other.Inversed();
		}

		Quaternion& operator/=(const Quaternion& Other)
		{
			return *this = *this / Other;
		}

		float LengthSqr() const
		{
			return X*X + Y*Y + Z*Z + W*W;
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
		* Convert quaternion into Euler angles (in degrees)
		*/
		Vector3 Euler() const
		{
			Vector3 angles;

			// roll (x-axis rotation)
			float sinr_cosp = 2 * (W * X + Y * Z);
			float cosr_cosp = 1 - 2 * (X * X + Y * Y);
			angles.X = std::atan2(sinr_cosp, cosr_cosp);

			// pitch (y-axis rotation)
			double sinp = 2 * (W * Y - Z * X);
			if (std::abs(sinp) >= 1)
				angles.Y = std::copysignf(M_PI / 2, sinp); // use 90 degrees if out of range
			else
				angles.Y = std::asinf(sinp);

			// yaw (z-axis rotation)
			float siny_cosp = 2 * (W * Z + X * Y);
			float cosy_cosp = 1 - 2 * (Y * Y + Z * Z);
			angles.Z = std::atan2(siny_cosp, cosy_cosp);

			angles.X = Math::Degrees(angles.X);
			angles.Y = Math::Degrees(angles.Y);
			angles.Z = Math::Degrees(angles.Z);

			return angles;
		}

		Quaternion Inversed() const
		{
			return -(*this) * (1 / LengthSqr());
		}

		Quaternion& Inverse()
		{
			return *this = Inversed();
		}

		~Quaternion() {}
	};

}
