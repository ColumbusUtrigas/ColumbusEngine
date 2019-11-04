#pragma once

#include <Core/Random.h>
#include <Math/Vector2.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	template <typename Type>
	struct Vector3_t;

	typedef Vector3_t<float> Vector3;
	typedef Vector3_t<double> dVector3;
	typedef Vector3_t<int> iVector3;
	typedef Vector3_t<bool> bVector3;

	template <typename Type>
	struct Vector3_t
	{
		Type X = 0;
		Type Y = 0;
		Type Z = 0;

		Vector3_t() {}
		Vector3_t(const Vector3_t& Base) : X(Base.X), Y(Base.Y), Z(Base.Z) {}
		Vector3_t(Vector3_t&& Base) : X(Base.X), Y(Base.Y), Z(Base.Z) {}
		Vector3_t(const Type& Scalar) : X(Scalar), Y(Scalar), Z(Scalar) {}
		Vector3_t(const Type& InX, const Type& InY, const Type& InZ) : X(InX), Y(InY), Z(InZ) {}
		Vector3_t(const Vector2_t<Type>& A, const Type& B) : X(A.X), Y(A.Y), Z(B) {}
		Vector3_t(const Type& A, const Vector2_t<Type>& B) : X(A), Y(B.X), Z(B.Y) {}

		Vector2_t<Type> XX() const { return Vector2_t<Type>(X, X); }
		Vector2_t<Type> XY() const { return Vector2_t<Type>(X, Y); }
		Vector2_t<Type> XZ() const { return Vector2_t<Type>(X, Z); }
		Vector2_t<Type> YX() const { return Vector2_t<Type>(Y, X); }
		Vector2_t<Type> YY() const { return Vector2_t<Type>(Y, Y); }
		Vector2_t<Type> YZ() const { return Vector2_t<Type>(Y, Z); }
		Vector2_t<Type> ZX() const { return Vector2_t<Type>(Z, X); }
		Vector2_t<Type> ZY() const { return Vector2_t<Type>(Z, Y); }
		Vector2_t<Type> ZZ() const { return Vector2_t<Type>(Z, Z); }

		Vector3_t<Type> XXX() const { return Vector3_t<Type>(X, X, X); }
		Vector3_t<Type> XXY() const { return Vector3_t<Type>(X, X, Y); }
		Vector3_t<Type> XXZ() const { return Vector3_t<Type>(X, X, Z); }
		Vector3_t<Type> XYX() const { return Vector3_t<Type>(X, Y, X); }
		Vector3_t<Type> XYY() const { return Vector3_t<Type>(X, Y, Y); }
		Vector3_t<Type> XYZ() const { return Vector3_t<Type>(X, Y, Z); }
		Vector3_t<Type> XZX() const { return Vector3_t<Type>(X, Z, X); }
		Vector3_t<Type> XZY() const { return Vector3_t<Type>(X, Z, Y); }
		Vector3_t<Type> XZZ() const { return Vector3_t<Type>(X, Z, Z); }
		Vector3_t<Type> YXX() const { return Vector3_t<Type>(Y, X, X); }
		Vector3_t<Type> YXY() const { return Vector3_t<Type>(Y, X, Y); }
		Vector3_t<Type> YXZ() const { return Vector3_t<Type>(Y, X, Z); }
		Vector3_t<Type> YYX() const { return Vector3_t<Type>(Y, Y, X); }
		Vector3_t<Type> YYY() const { return Vector3_t<Type>(Y, Y, Y); }
		Vector3_t<Type> YYZ() const { return Vector3_t<Type>(Y, Y, Z); }
		Vector3_t<Type> YZX() const { return Vector3_t<Type>(Y, Z, X); }
		Vector3_t<Type> YZY() const { return Vector3_t<Type>(Y, Z, Y); }
		Vector3_t<Type> YZZ() const { return Vector3_t<Type>(Y, Z, Z); }
		Vector3_t<Type> ZXX() const { return Vector3_t<Type>(Z, X, X); }
		Vector3_t<Type> ZXY() const { return Vector3_t<Type>(Z, X, Y); }
		Vector3_t<Type> ZXZ() const { return Vector3_t<Type>(Z, X, Z); }
		Vector3_t<Type> ZYX() const { return Vector3_t<Type>(Z, Y, X); }
		Vector3_t<Type> ZYY() const { return Vector3_t<Type>(Z, Y, Y); }
		Vector3_t<Type> ZYZ() const { return Vector3_t<Type>(Z, Y, Z); }
		Vector3_t<Type> ZZX() const { return Vector3_t<Type>(Z, Z, X); }
		Vector3_t<Type> ZZY() const { return Vector3_t<Type>(Z, Z, Y); }
		Vector3_t<Type> ZZZ() const { return Vector3_t<Type>(Z, Z, Z); }

		template <typename T>
		operator Vector3_t<T>() const { return Vector3_t<T>((T)X, (T)Y, (T)Z); }

		Vector3_t& operator=(const Vector3_t& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			return *this;
		}

		Vector3_t& operator=(Vector3_t&& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			return *this;
		}

		Vector3_t operator+() const
		{
			return *this;
		}

		Vector3_t operator+(const Type& Scalar) const
		{
			return Vector3_t(X + Scalar, Y + Scalar, Z + Scalar);
		}

		friend Vector3_t operator+(const Type& Scalar, const Vector3_t& Other)
		{
			return Other + Scalar;
		}

		Vector3_t operator+(const Vector3_t& Other) const
		{
			return Vector3_t(X + Other.X, Y + Other.Y, Z + Other.Z);
		}

		Vector3_t operator-() const
		{
			return Vector3_t(-X, -Y, -Z);
		}

		Vector3_t operator-(const Type& Scalar) const
		{
			return Vector3_t(X - Scalar, Y - Scalar, Z - Scalar);
		}

		friend Vector3_t operator-(const Type& Scalar, const Vector3_t& Other)
		{
			return Vector3_t(Scalar - Other.X, Scalar - Other.Y, Scalar - Other.Z);
		}

		Vector3_t operator-(const Vector3_t& Other) const
		{
			return Vector3_t(X - Other.X, Y - Other.Y, Z - Other.Z);
		}

		Vector3_t operator*(const Type& Scalar) const
		{
			return Vector3_t(X * Scalar, Y * Scalar, Z * Scalar);
		}

		friend Vector3_t operator*(const Type& Scalar, const Vector3_t& Other)
		{
			return Other * Scalar;
		}

		Vector3_t operator*(const Vector3_t& Other) const
		{
			return Vector3_t(X * Other.X, Y * Other.Y, Z * Other.Z);
		}

		Vector3_t operator/(const Type& Scalar) const
		{
			return Vector3_t(X / Scalar, Y / Scalar, Z / Scalar);
		}

		friend Vector3_t operator/(const Type& Scalar, const Vector3_t& Other)
		{
			return Vector3_t(Scalar / Other.X, Scalar / Other.Y, Scalar / Other.Z);
		}

		Vector3_t operator/(const Vector3_t& Other) const
		{
			return Vector3_t(X / Other.X, Y / Other.Y, Z / Other.Z);
		}

		Vector3_t& operator+=(const Type& Scalar)
		{
			return *this = *this + Scalar;
		}

		Vector3_t& operator+=(const Vector3_t& Other)
		{
			return *this = *this + Other;
		}

		Vector3_t& operator-=(const Type& Scalar)
		{
			return *this = *this - Scalar;
		}

		Vector3_t& operator-=(const Vector3_t& Other)
		{
			return *this = *this - Other;
		}

		Vector3_t& operator*=(const Type& Scalar)
		{
			return *this = *this * Scalar;
		}

		Vector3_t& operator*=(const Vector3_t& Other)
		{
			return *this = *this * Other;
		}

		Vector3_t& operator/=(const Type& Scalar)
		{
			return *this = *this / Scalar;
		}

		Vector3_t& operator/=(const Vector3_t& Other)
		{
			return *this = *this / Other;
		}

		bool operator==(const Vector3_t& Other) const
		{
			return X == Other.X && Y == Other.Y && Z == Other.Z;
		}

		bool operator!=(const Vector3_t& Other) const
		{
			return !(*this == Other);
		}

		static Vector3_t Random(const Vector3_t& Min, const Vector3_t& Max)
		{
			Vector3_t Result;

			Result.X = Random::Range(Min.X, Max.X);
			Result.Y = Random::Range(Min.Y, Max.Y);
			Result.Z = Random::Range(Min.Z, Max.Z);

			return Result;
		}

		static Vector3_t Clamp(const Vector3_t& A, const Vector3_t& Min, const Vector3_t& Max)
		{
			return Vector3_t(Math::Clamp(A.X, Min.X, Max.X), Math::Clamp(A.Y, Min.Y, Max.Y), Math::Clamp(A.Z, Min.Z, Max.Z));
		}

		static Vector3_t Normalize(const Vector3_t& Other)
		{
			return Other * (1.0f / sqrtf(Other.X * Other.X + Other.Y * Other.Y + Other.Z * Other.Z));
		}

		static Type Dot(const Vector3_t& V1, const Vector3_t& V2)
		{
			return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
		}

		static Vector3_t Cross(const Vector3_t& V1, const Vector3_t& V2)
		{
			return Vector3_t(V1.Y * V2.Z - V1.Z * V2.Y, V1.Z * V2.X - V1.X * V2.Z, V1.X * V2.Y - V1.Y * V2.X);
		}

		Vector3_t Clamped(const Vector3_t& Min, const Vector3_t& Max) const
		{
			return Vector3_t(Math::Clamp(X, Min.X, Max.X), Math::Clamp(Y, Min.Y, Max.Y), Math::Clamp(Z, Min.Z, Max.Z));
		}

		Vector3_t& Clamp(const Vector3_t& Min, const Vector3_t& Max)
		{
			return *this = Clamped(Min, Max);
		}

		Vector3_t Normalized() const
		{
			return *this * (1.0f / sqrtf(X * X + Y * Y + Z * Z));
		}

		Vector3_t& Normalize()
		{
			return *this = Normalized();
		}

		Type Length(const Vector3_t& Other) const
		{
			return sqrtf(Math::Sqr(Other.X - X) + Math::Sqr(Other.Y - Y) + Math::Sqr(Other.Z - Z));
		}

		Type LengthSquare(const Vector3_t& Other) const
		{
			return Math::Sqr(Other.X - X) + Math::Sqr(Other.Y - Y) + Math::Sqr(Other.Z - Z);
		}

		Type Dot(const Vector3_t& Other) const
		{
			return X * Other.X + Y * Other.Y + Z * Other.Z;
		}

		Vector3_t Cross(const Vector3_t& Other) const
		{
			return Vector3_t(Y * Other.Z - Z * Other.Y, Z * Other.X - X * Other.Z, X * Other.Y - Y * Other.X);
		}
	};

}



