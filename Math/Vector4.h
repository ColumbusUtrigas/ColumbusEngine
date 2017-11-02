/************************************************
*                   Vector4.h                   *
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
#include <System/Random.h>

namespace Columbus
{

	class C_Vector4
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		float w = 0;
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector4() {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector4(float aX, float aY, float aZ, float aW) : x(aX), y(aY), z(aZ), w(aW) {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector4(glm::vec4 aVec) : x(aVec.x), y(aVec.y), z(aVec.z), w(aVec.w) {}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec4 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
			w = aVec.w;
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		inline glm::vec4 toGLM()
		{
			return glm::vec4(x, y, z, w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		inline C_Vector4& operator=(C_Vector4 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			w = aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline C_Vector4 operator+(const C_Vector4 aOther)
		{
			return C_Vector4(x + aOther.x, y + aOther.y, z + aOther.z, w + aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline C_Vector4 operator-(const C_Vector4 aOther)
		{
			return C_Vector4(x - aOther.x, y - aOther.y, z - aOther.z, w - aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline C_Vector4 operator-()
		{
			return C_Vector4(-x, -y, -z, -w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector4 operator*(const C_Vector4 aOther)
		{
			return C_Vector4(x * aOther.x, y * aOther.y, z * aOther.z, w * aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector4 operator*(const float aOther)
		{
			return C_Vector4(x * aOther, y * aOther, z * aOther, w * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline C_Vector4 operator/(const C_Vector4 aOther)
		{
			return C_Vector4(x / aOther.x, y / aOther.y, z / aOther.z, w / aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline C_Vector4 operator/(const float aOther)
		{
			return C_Vector4(x / aOther, y / aOther, z / aOther, w / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const C_Vector4 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const C_Vector4 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z && w != aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector4 operator+=(const C_Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector4 operator-=(const C_Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector4 operator*=(const C_Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector4 operator/=(const C_Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector4 operator/=(const float aOther)
		{
			x /= aOther;
			y /= aOther;
			z /= aOther;
			w /= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector4 random(const C_Vector4 aMin, const C_Vector4 aMax)
		{
			C_Vector4 ret;
			ret.x = C_Random::range(aMin.x, aMax.x);
			ret.y = C_Random::range(aMin.y, aMax.y);
			ret.z = C_Random::range(aMin.z, aMax.z);
			ret.w = C_Random::range(aMin.w, aMax.w);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		inline float length()
		{
			return sqrt((x * x) + (y * y) + (z * z) + (w * w));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between two vectors
		inline float length(const C_Vector4 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2) + pow(aVec.z - z, 2) + pow(aVec.w - w, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		inline C_Vector4 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z) + (w * w));
			return C_Vector4(x / l, y / l, z / l, w / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		inline ~C_Vector4() {}
	};

	typedef C_Vector4 vec4;

}
