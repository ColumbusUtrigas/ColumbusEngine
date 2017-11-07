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

namespace Columbus
{

	class C_Vector2;
	typedef C_Vector2 vec2;

	class C_Vector2
	{
	public:
		float x = 0;
		float y = 0;
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2() :
			x(0),
			y(0)
		{}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2(const float aX, const float aY) :
			x(static_cast<float>(aX)),
			y(static_cast<float>(aY))
		{}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2(const glm::vec2 aVec) :
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
		inline C_Vector2& operator=(C_Vector2 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline C_Vector2 operator+(const C_Vector2 aOther)
		{
			return C_Vector2(x + aOther.x, y + aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline C_Vector2 operator-(const C_Vector2 aOther)
		{
			return C_Vector2(x - aOther.x, y - aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline C_Vector2 operator-()
		{
			return C_Vector2(-x, -y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector2 operator*(const C_Vector2 aOther)
		{
			return C_Vector2(x * aOther.x, y * aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline C_Vector2 operator*(const float aOther)
		{
			return C_Vector2(x * aOther, y * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *
		inline friend C_Vector2 operator*(float aL, const C_Vector2 aR)
		{
			return C_Vector2(aL * aR.x, aL * aR.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline C_Vector2 operator/(const C_Vector2 aOther)
		{
			return C_Vector2(x / aOther.x, y / aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline C_Vector2 operator/(const float aOther)
		{
			return C_Vector2(x / aOther, y / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const C_Vector2 aOther)
		{
			return (x == aOther.x && y == aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const C_Vector2 aOther)
		{
			return (x != aOther.x && y != aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector2 operator+=(const C_Vector2 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector2 operator-=(const C_Vector2 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector2 operator*=(const C_Vector2 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector2 operator/=(const C_Vector2 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector2 operator/=(const float aOther)
		{
			x /= aOther;
			y /= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector2 random(const C_Vector2 aMin, const C_Vector2 aMax)
		{
			C_Vector2 ret;
			ret.x = C_Random::range(aMin.x, aMax.x);
			ret.y = C_Random::range(aMin.y, aMax.y);
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
		inline float length(const C_Vector2 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		inline C_Vector2 normalize()
		{
			float l = sqrt((x * x) + (y * y));
			return C_Vector2(x / l, y / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline float dot(C_Vector2 aOther)
		{
			return x * aOther.x + y * aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return dot product of 2 vectors
		inline static float dot(C_Vector2 aV1, C_Vector2 aV2)
		{
			return aV1.x * aV2.x + aV1.y * aV2.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return cross product of 2 vectors
		inline C_Vector2 cross(C_Vector2 aOther)
		{
			C_Vector2 ret;
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		inline ~C_Vector2() {}
	};

}






