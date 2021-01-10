#pragma once

#include <Core/Random.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	template <typename Type>
	struct Vector2_t;

	typedef Vector2_t<float> Vector2;
	typedef Vector2_t<double> dVector2;
	typedef Vector2_t<int> iVector2;
	typedef Vector2_t<bool> bVector2;

	template <typename Type>
	struct Vector2_t
	{
		Type X = 0;
		Type Y = 0;

		Vector2_t() {}
		Vector2_t(const Vector2_t& Base) : X(Base.X), Y(Base.Y) {}
		Vector2_t(Vector2_t&& Base) : X(Base.X), Y(Base.Y) {}
		Vector2_t(const Type& Scalar) : X(Scalar), Y(Scalar) {}
		Vector2_t(const Type& InX, const Type& InY) : X(InX), Y(InY) {}

		Vector2_t<Type> XX() const { return Vector2_t<Type>(X, X); }
		Vector2_t<Type> XY() const { return Vector2_t<Type>(X, Y); }
		Vector2_t<Type> YX() const { return Vector2_t<Type>(Y, X); }
		Vector2_t<Type> YY() const { return Vector2_t<Type>(Y, Y); }

		template <typename T>
		operator Vector2_t<T>() const { return Vector2_t<T>((T)X, (T)Y); }

		Vector2_t& operator=(const Vector2_t& Other)
		{
			X = Other.X;
			Y = Other.Y;
			return *this;
		}

		Vector2_t& operator=(Vector2_t&& Other)
		{
			X = Other.X;
			Y = Other.Y;
			return *this;
		}

		Vector2_t operator+() const
		{
			return *this;
		}

		Vector2_t operator+(const Type& Scalar) const
		{
			return Vector2_t(X + Scalar, Y + Scalar);
		}

		friend Vector2_t operator+(const Type& Scalar, const Vector2_t& Other)
		{
			return Other + Scalar;
		}

		Vector2_t operator+(const Vector2_t& Other) const
		{
			return Vector2_t(X + Other.X, Y + Other.Y);
		}

		Vector2_t operator-() const
		{
			return Vector2_t(-X, -Y);
		}

		Vector2_t operator-(const Type& Scalar) const
		{
			return Vector2_t(X - Scalar, Y - Scalar);
		}

		friend Vector2_t operator-(const Type& Scalar, const Vector2_t& Other)
		{
			return Vector2_t(Scalar - Other.X, Scalar - Other.Y);
		}

		Vector2_t operator-(const Vector2_t& Other) const
		{
			return Vector2_t(X - Other.X, Y - Other.Y);
		}

		Vector2_t operator*(const Type& Scalar) const
		{
			return Vector2_t(X * Scalar, Y * Scalar);
		}

		friend Vector2_t operator*(const Type& Scalar, const Vector2_t& Other)
		{
			return Other * Scalar;
		}

		Vector2_t operator*(const Vector2_t& Other) const
		{
			return Vector2_t(X * Other.X, Y * Other.Y);
		}

		Vector2_t operator/(const Type& Scalar) const
		{
			return Vector2_t(X / Scalar, Y / Scalar);
		}

		friend Vector2_t operator/(const Type& Scalar, const Vector2_t& Other)
		{
			return Vector2_t(Scalar / Other.X, Scalar / Other.Y);
		}

		Vector2_t operator/(const Vector2_t& Other) const
		{
			return Vector2_t(X / Other.X, Y / Other.Y);
		}

		Vector2_t& operator+=(const Type& Scalar)
		{
			return *this = *this + Scalar;
		}

		Vector2_t& operator+=(const Vector2_t& Other)
		{
			return *this = *this + Other;
		}

		Vector2_t& operator-=(const Type& Scalar)
		{
			return *this = *this - Scalar;
		}

		Vector2_t& operator-=(const Vector2_t& Other)
		{
			return *this = *this - Other;
		}

		Vector2_t& operator*=(const Type& Scalar)
		{
			return *this = *this * Scalar;
		}

		Vector2_t& operator*=(const Vector2_t& Other)
		{
			return *this = *this * Other;
		}

		Vector2_t& operator/=(const Type& Scalar)
		{
			return *this = *this / Scalar;
		}

		Vector2_t& operator/=(const Vector2_t& Other)
		{
			return *this = *this / Other;
		}

		bool operator==(const Vector2_t& Other) const
		{
			return X == Other.X && Y == Other.Y;
		}

		bool operator!=(const Vector2_t& Other) const
		{
			return !(*this == Other);
		}

		static Vector2_t Random(const Vector2_t& Min, const Vector2_t& Max)
		{
			Vector2_t Result;

			Result.X = Random::Range(Min.X, Max.X);
			Result.Y = Random::Range(Min.Y, Max.Y);

			return Result;
		}

		Vector2_t Clamped(const Vector2_t& Min, const Vector2_t& Max) const
		{
			return Vector2_t(Math::Clamp(X, Min.X, Max.X), Math::Clamp(Y, Min.Y, Max.Y));
		}

		Vector2_t& Clamp(const Vector2_t& Min, const Vector2_t& Max)
		{
			return *this = Clamped(Min, Max);
		}

		Vector2_t Normalized() const
		{
			return *this * (1.0f / sqrtf(X * X + Y * Y));
		}

		Vector2_t& Normalize() const
		{
			return *this = Normalized();
		}

		Type Length(const Vector2_t& Other) const
		{
			return sqrtf(Math::Sqr(Other.X - X) + Math::Sqr(Other.Y - Y));
		}

		Type LengthSquare(const Vector2_t& Other) const
		{
			return Math::Sqr(Other.X - X) + Math::Sqr(Other.Y - Y);
		}

		Type Dot(const Vector2_t& Other) const
		{
			return X * Other.X + Y * Other.Y;
		}
	};

}
