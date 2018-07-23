#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <System/Random.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	class Vector2;
	typedef Vector2 vec2;

	class Vector2
	{
	public:
		float X = 0.0f;
		float Y = 0.0f;
	public:
		explicit Vector2() :
			X(0.0f),
			Y(0.0f)
		{ }

		Vector2(const Vector2& Base) :
			X(Base.X),
			Y(Base.Y)
		{ }

		Vector2(Vector2&& Base) noexcept :
			X(std::move(Base.X)),
			Y(std::move(Base.Y))
		{ }
		
		Vector2(float InX, float InY) :
			X(InX),
			Y(InY)
		{ }
		
		explicit Vector2(glm::vec2 InVec) :
			X(InVec.x),
			Y(InVec.y)
		{ }
		/*
		* Covert from GLM vec2 to Columbus Vector2
		*/
		void FromGLM(glm::vec2 InVec)
		{
			X = InVec.x;
			Y = InVec.y;
		}
		/*
		* Covert from Columbus Vector2 to GLM vec2
		*/
		glm::vec2 ToGLM()
		{
			return glm::vec2(X, Y);
		}
		/*
		* Operator= for equaling to vectors
		* @return Vector2&: *this
		*/
		inline Vector2& operator=(const Vector2 Other)
		{
			X = Other.X;
			Y = Other.Y;
			return *this;
		}
		/*
		* Operator= for summing two vectors
		* Summing component to component
		*/
		inline Vector2 operator+(const Vector2 Other)
		{
			return Vector2(X + Other.X, Y + Other.Y);
		}
		/*
		* Operator- for subtraction two vectors
		* Subtract component by component
		*/
		inline Vector2 operator-(const Vector2 Other)
		{
			return Vector2(X - Other.X, Y - Other.Y);
		}
		/*
		* Operator- for negation of vector
		*/
		inline Vector2 operator-()
		{
			return Vector2(-X, -Y);
		}
		/*
		* Operator* for multiplying two vectors
		*/
		inline Vector2 operator*(const Vector2 Other)
		{
			return Vector2(X * Other.X, Y * Other.Y);
		}
		/*
		* Operator* for multiplying vector by scalar
		*/
		inline Vector2 operator*(const float Other)
		{
			return Vector2(X * Other, Y * Other);
		}
		/*
		* Operator* for multiplying scalar with vector
		*/
		inline friend Vector2 operator*(const float L, const Vector2 R)
		{
			return Vector2(L * R.X, L * R.Y);
		}
		/*
		* Operator/ for division vector by vector
		*/
		inline Vector2 operator/(const Vector2 Other)
		{
			return Vector2(X / Other.X, Y / Other.Y);
		}
		/*
		* Operator/ for division vector by scalar
		*/
		inline Vector2 operator/(const float Other)
		{
			const float Scalar = 1.0f / Other;
			return Vector2(X * Scalar, Y * Scalar);
		}
		/*
		* Operator== for comparison two vectors
		*/
		inline bool operator==(const Vector2 Other)
		{
			return (X == Other.X && Y == Other.Y);
		}
		/*
		* Operator!= for comparison two vectors
		*/
		inline bool operator!=(const Vector2 Other)
		{
			return !(*this == Other);
		}
		/*
		* Operator+= for summing two vectors
		*/
		inline Vector2 operator+=(const Vector2 Other)
		{
			X += Other.X;
			Y += Other.Y;
			return *this;
		}
		/*
		* Operator-= for subtraction two vectors
		*/
		inline Vector2 operator-=(const Vector2 Other)
		{
			X -= Other.X;
			Y -= Other.Y;
			return *this;
		}
		/*
		* Operator*= for multiplying two vectors
		*/
		inline Vector2 operator*=(const Vector2 Other)
		{
			X *= Other.X;
			Y *= Other.Y;
			return *this;
		}
		/*
		* Operator*= for multiplying vector with scalar
		*/
		inline Vector2 operator*=(const float Other)
		{
			X *= Other;
			Y *= Other;
			return *this;
		}
		/*
		* Operator=/ for division vector by vector
		*/
		inline Vector2 operator/=(const Vector2 Other)
		{
			X /= Other.X;
			Y /= Other.Y;
			return *this;
		}
		/*
		* Operator/= for division vector by scalar
		*/
		inline Vector2 operator/=(const float Other)
		{
			const float Scalar = 1.0f / Other;
			X *= Scalar;
			Y *= Scalar;
			return *this;
		}
		/*
		* Return random vector between
		*/
		inline static Vector2 Random(const Vector2 Min, const Vector2 MaX)
		{
			Vector2 ret;
			ret.X = Random::range(Min.X, MaX.X);
			ret.Y = Random::range(Min.Y, MaX.Y);
			return ret;
		}
		/*
		* Return length of this vector
		*/
		inline float Length()
		{
			return sqrt((X * X) + (Y * Y));
		}
		/*
		* Return length between this vector and Vec
		*/
		inline float Length(const Vector2 Vec)
		{
			return sqrt(pow(Vec.X - X, 2) + pow(Vec.Y - Y, 2));
		}
		/*
		* Return normalized vector
		*/
		inline Vector2 Normalize()
		{
			float l = Math::Sqrt((X * X) + (Y * Y));
			float S = 1.0f / l;
			return Vector2(X * S, Y * S);
		}
		/*
		* Dot product of two vectors
		*/
		inline float Dot(const Vector2 Other)
		{
			return X * Other.X + Y * Other.Y;
		}
		/*
		* Dot product of two vectors
		*/
		inline static float Dot(const Vector2 V1, const Vector2 V2)
		{
			return V1.X * V2.X + V1.Y * V2.Y;
		}
		/*
		* Clamp each component of this vector
		*/
		inline static Vector2 Clamp(Vector2 A, const Vector2 Min, const Vector2 Max)
		{
			A.X = Math::Clamp(A.X, Min.X, Max.X);
			A.Y = Math::Clamp(A.Y, Min.Y, Max.Y);
			return A;
		}

		~Vector2() { }
	};

}






