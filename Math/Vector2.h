/************************************************
*                   Vector2.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*                Nika(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

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
		float x = 0;
		float y = 0;
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector2() :
			x(0),
			y(0)
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector2(const float aX, const float aY) :
			x(static_cast<float>(aX)),
			y(static_cast<float>(aY))
		{}
		////////////////////////////////////////////////////////////////////////////
		inline explicit Vector2(const glm::vec2 aVec) :
			x(static_cast<float>(aVec.x)),
			y(static_cast<float>(aVec.y))
		{}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec2 aVec)
		{
			x = static_cast<float>(aVec.x);
			y = static_cast<float>(aVec.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		inline glm::vec2 toGLM()
		{
			return glm::vec2(x, y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		inline Vector2& operator=(Vector2 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline Vector2 operator+(const Vector2 aOther)
		{
			return Vector2(x + aOther.x, y + aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline Vector2 operator-(const Vector2 aOther)
		{
			return Vector2(x - aOther.x, y - aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline Vector2 operator-()
		{
			return Vector2(-x, -y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline Vector2 operator*(const Vector2 aOther)
		{
			return Vector2(x * aOther.x, y * aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline Vector2 operator*(const float aOther)
		{
			return Vector2(x * aOther, y * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline friend Vector2 operator*(float aL, const Vector2 aR)
		{
			return Vector2(aL * aR.x, aL * aR.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline Vector2 operator/(const Vector2 aOther)
		{
			return Vector2(x / aOther.x, y / aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline Vector2 operator/(const float aOther)
		{
			return Vector2(x / aOther, y / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const Vector2 aOther)
		{
			return (x == aOther.x && y == aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const Vector2 aOther)
		{
			return (x != aOther.x && y != aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline Vector2 operator+=(const Vector2 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline Vector2 operator-=(const Vector2 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline Vector2 operator*=(const Vector2 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline Vector2 operator*=(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline Vector2 operator/=(const Vector2 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline Vector2 operator/=(const float aOther)
		{
			const float Scalar = 1.0f / aOther;
			x *= Scalar;
			y *= Scalar;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static Vector2 random(const Vector2 aMin, const Vector2 aMax)
		{
			Vector2 ret;
			ret.x = Random::range(aMin.x, aMax.x);
			ret.y = Random::range(aMin.y, aMax.y);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		inline float length()
		{
			return sqrt((x * x) + (y * y));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between 2 vectors
		inline float length(const Vector2 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalized vector
		inline Vector2 normalize()
		{
			float l = sqrt((x * x) + (y * y));
			return Vector2(x / l, y / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline float dot(Vector2 aOther)
		{
			return x * aOther.x + y * aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline static float dot(Vector2 aV1, Vector2 aV2)
		{
			return aV1.x * aV2.x + aV1.y * aV2.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return cross product of 2 vectors
		inline Vector2 cross(Vector2 aOther)
		{
			Vector2 ret;
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		inline static Vector2 clamp(Vector2 a, Vector2 aMin, Vector2 aMax)
		{
			a.x = Clamp(a.x, aMin.x, aMax.x);
			a.y = Clamp(a.y, aMin.y, aMax.y);
			return a;
		}
		////////////////////////////////////////////////////////////////////////////
		inline ~Vector2() {}
	};

}






