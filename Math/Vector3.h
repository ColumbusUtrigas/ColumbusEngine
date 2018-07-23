#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <System/Random.h>
#include <Math/MathUtil.h>

namespace Columbus
{

	class Vector3;
	typedef Vector3 vec3;

	class Vector3
	{
	public:
		float X = 0;
		float Y = 0;
		float Z = 0;
	public:
		constexpr explicit Vector3() :
			X(0.0f),
			Y(0.0f),
			Z(0.0f)
		{ }

		constexpr Vector3 (const Vector3& Base) :
			X(Base.X),
			Y(Base.Y),
			Z(Base.Z)
		{ }

		constexpr Vector3 (Vector3&& Base) noexcept :
			X(std::move(Base.X)),
			Y(std::move(Base.Y)),
			Z(std::move(Base.Z))
		{ }
		
		constexpr Vector3(float InX, float InY, float InZ) :
			X(InX),
			Y(InY),
			Z(InZ)
		{ }
		
		explicit Vector3(glm::vec3 InVec) :
			X(InVec.x),
			Y(InVec.y),
			Z(InVec.z)
		{ }
		/*
		* Convert from GLM vec3 to Columbus Vector3
		*/
		inline void FromGLM(glm::vec3 InVec)
		{
			X = InVec.x;
			Y = InVec.y;
			Z = InVec.z;
		}
		/*
		* Convert from Columbus Vector3 to GLM vec3
		*/
		inline glm::vec3 ToGLM()
		{
			return glm::vec3(X, Y, Z);
		}
		/*
		* Operator= for equaling to vectors
		* @return Vector3&: *this
		*/
		inline Vector3& operator=(const Vector3 Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			return *this;
		}
		/*
		* Operator= for summing two vectors
		* Summing component to component
		*/
		inline Vector3 operator+(const Vector3 Other) const
		{
			return Vector3(X + Other.X, Y + Other.Y, Z + Other.Z);
		}
		/*
		* Operator- for subtraction two vectors
		* Subtract component by component
		*/
		inline Vector3 operator-(const Vector3 Other) const
		{
			return Vector3(X - Other.X, Y - Other.Y, Z - Other.Z);
		}
		/*
		* Operator- for negation of vector
		*/
		inline Vector3 operator-() const
		{
			return Vector3(-X, -Y, -Z);
		}
		/*
		* Operator* for multiplying two vectors
		*/
		inline Vector3 operator*(const Vector3 Other) const
		{
			return Vector3(X * Other.X, Y * Other.Y, Z * Other.Z);
		}
		/*
		* Operator* for multiplying vector by scalar
		*/
		inline Vector3 operator*(const float Other) const
		{
			return Vector3(X * Other, Y * Other, Z * Other);
		}
		/*
		* Operator* for multiplying scalar with vector
		*/
		inline friend Vector3 operator*(float L, const Vector3 R)
		{
			return Vector3(L * R.X, L * R.Y, L * R.Z);
		}
		/*
		* Operator/ for division vector by vector
		*/
		inline Vector3 operator/(const Vector3 Other) const
		{
			return Vector3(X / Other.X, Y / Other.Y, Z / Other.Z);
		}
		/*
		* Operator/ for division vector by scalar
		*/
		inline Vector3 operator/(const float Other) const
		{
			const float Scalar = 1.0f / Other;
			return Vector3(X * Scalar, Y * Scalar, Z * Scalar);
		}
		/*
		* Operator== for comparison two vectors
		*/
		inline bool operator==(const Vector3 Other) const
		{
			return (X == Other.X && Y == Other.Y && Z == Other.Z);
		}
		/*
		* Operator!= for comparison two vectors
		*/
		inline bool operator!=(const Vector3 Other) const
		{
			return (X != Other.X && Y != Other.Y && Z != Other.Z);
		}
		/*
		* Operator+= for summing two vectors
		*/
		inline Vector3& operator+=(const Vector3 Other)
		{
			*this = *this + Other;
			return *this;
		}
		/*
		* Operator-= for subtraction two vectors
		*/
		inline Vector3& operator-=(const Vector3 Other)
		{
			*this = *this - Other;
			return *this;
		}
		/*
		* Operator*= for multiplying two vectors
		*/
		inline Vector3& operator*=(const Vector3 Other)
		{
			*this = *this * Other;
			return *this;
		}
		/*
		* Operator*= for multiplying vector with scalar
		*/
		inline Vector3& operator*=(const float Other)
		{
			*this = *this * Other;
			return *this;
		}
		/*
		* Operator=/ for division vector by vector
		*/
		inline Vector3& operator/=(const Vector3 Other)
		{
			*this = *this / Other;
			return *this;
		}
		/*
		* Operator/= for division vector by scalar
		*/
		inline Vector3& operator/=(const float Other)
		{
			*this = *this / Other;
			return *this;
		}
		/*
		* Return random vector between
		*/
		inline static Vector3 Random(const Vector3 Min, const Vector3 Max)
		{
			Vector3 Result;
			Result.X = Random::range(Min.X, Max.X);
			Result.Y = Random::range(Min.Y, Max.Y);
			Result.Z = Random::range(Min.Z, Max.Z);
			return Result;
		}
		/*
		* Return length of this vector
		*/
		inline float Length()
		{
			return Math::Sqrt((X * X) + (Y * Y) + (Z * Z));
		}
		/*
		* Return length between this vector and Vec
		*/
		inline float Length(const Vector3 InVec)
		{
			return sqrt(pow(InVec.X - X, 2) + pow(InVec.Y - Y, 2) + pow(InVec.Z - Z, 2));
		}
		/*
		* Return normalized vector
		*/
		inline Vector3 Normalize() const
		{
			return Normalize(*this);
		}
		/*
		* Return normalized vector
		*/
		inline static Vector3 Normalize(Vector3 Vec)
		{
			const float L = Math::Sqrt((Vec.X * Vec.X) + (Vec.Y * Vec.Y) + (Vec.Z * Vec.Z));
			const float Scalar = 1.0f / L;
			return Vector3(Vec * Scalar);
		}
		/*
		* Dot product of two vectors
		*/
		inline float Dot(const Vector3 Other)
		{
			return X * Other.X + Y * Other.Y + Z * Other.Z;
		}
		/*
		* Dot product of two vectors
		*/
		inline static float Dot(const Vector3 V1, const Vector3 V2)
		{
			return V1.X * V2.X + V1.Y * V2.Y + V1.Z * V2.Z;
		}
		/*
		* Cross product of two vectors
		*/
		inline Vector3 Cross(Vector3 Other)
		{
			return Cross(*this, Other);
		}
		/*
		* Cross product of two vectors
		*/
		inline static Vector3 Cross(Vector3 V1, Vector3 V2)
		{
			Vector3 Result;
			Result.X = V1.Y * V2.Z - V1.Z * V2.Y;
			Result.Y = V1.Z * V2.X - V1.X * V2.Z;
			Result.Z = V1.X * V2.Y - V1.Y * V2.X;
			return Result;
		}
		/*
		* Clamp each component of vector
		*/
		inline static Vector3 Clamp(Vector3 A, const Vector3 Min, const Vector3 Max)
		{
			A.X = Math::Clamp(A.X, Min.X, Max.X);
			A.Y = Math::Clamp(A.Y, Min.Y, Max.Y);
			A.Z = Math::Clamp(A.Z, Min.Z, Max.Z);
			return A;
		}
		/*
		* Clamp each component of this vector
		*/
		inline Vector3 Clamp(const Vector3 Min, const Vector3 Max)
		{
			return Clamp(*this, Min, Max);
		}
		
		~Vector3() { }
	};

}









