/************************************************
*                   Vector2.h                   *
*************************************************
*          This file is a part of:              *
*               COLUMBUS ENGINE                 *
*************************************************
*             Nikolay(Columbus) Red             *
*                   20.07.2017                  *
*************************************************/

#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace C
{

	class C_Vector2
	{
	public:
		float x = 0;
		float y = 0;
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2() {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2(const float aX, const float aY) : x((float)aX), y((float)aY) {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector2(const glm::vec2 aVec) : x(aVec.x), y(aVec.y) {}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec2 aVec)
		{
			x = aVec.x;
			y = aVec.y;
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
		//Opeator /
		inline C_Vector2 operator/(const C_Vector2 aOther)
		{
			return C_Vector2(x / aOther.x, y / aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(C_Vector2 aOther)
		{
			return (x == aOther.x && y == aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(C_Vector2 aOther)
		{
			return (x != aOther.x && y != aOther.y);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector2 operator+=(C_Vector2 aOther)
		{
			x += aOther.x;
			y += aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector2 operator-=(C_Vector2 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector2 operator*=(C_Vector2 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector2 operator/=(C_Vector2 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector2 random(C_Vector2 aMin, C_Vector2 aMax)
		{
			C_Vector2 ret;
			ret.x = aMin.x + (float)(rand()) / ((float)(RAND_MAX / (aMax.x - aMin.x)));
			ret.y = aMin.y + (float)(rand()) / ((float)(RAND_MAX / (aMax.y - aMin.y)));
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
		inline float length(C_Vector2 aVec)
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
		//Destructor
		inline ~C_Vector2() {}
	};

	typedef C_Vector2 vec2;

}
