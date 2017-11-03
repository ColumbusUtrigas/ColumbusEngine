/************************************************
*                   Vector3.h                   *
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

	class C_Vector3
	{
	public:
		float x = 0;
		float y = 0;
		float z = 0;
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector3() {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector3(float aX, float aY, float aZ) : x(aX), y(aY), z(aZ) {}
		////////////////////////////////////////////////////////////////////////////
		//Constructor
		inline C_Vector3(const glm::vec3 aVec) : x(aVec.x), y(aVec.y), z(aVec.z) {}
		////////////////////////////////////////////////////////////////////////////
		//Conversion from GLM vector
		inline void fromGLM(const glm::vec3 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
		}
		////////////////////////////////////////////////////////////////////////////
		//Conversion to GLM vector
		inline glm::vec3 toGLM()
		{
			return glm::vec3(x, y, z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator =
		inline C_Vector3& operator=(C_Vector3 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator +
		inline C_Vector3 operator+(const C_Vector3 aOther)
		{
			return C_Vector3(x + aOther.x, y + aOther.y, z + aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline C_Vector3 operator-(const C_Vector3 aOther)
		{
			return C_Vector3(x - aOther.x, y - aOther.y, z - aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -
		inline C_Vector3 operator-()
		{
			return C_Vector3(-x, -y, -z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector3 operator*(const C_Vector3 aOther)
		{
			return C_Vector3(x * aOther.x, y * aOther.y, z * aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector3 operator*(const float aOther)
		{
			return C_Vector3(x * aOther, y * aOther, z * aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline C_Vector3 operator/(const C_Vector3 aOther)
		{
			return C_Vector3(x / aOther.x, y / aOther.y, z / aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /
		inline C_Vector3 operator/(const float aOther)
		{
			return C_Vector3(x / aOther, y / aOther, z / aOther);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(const C_Vector3 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(const C_Vector3 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector3 operator+=(const C_Vector3 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector3 operator-=(const C_Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector3 operator*=(const C_Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector3 operator/=(const C_Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector3 operator/=(const float aOther)
		{
			x /= aOther;
			y /= aOther;
			z /= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector3 random(const C_Vector3 aMin, const C_Vector3 aMax)
		{
			C_Vector3 ret;
			ret.x = C_Random::range(aMin.x, aMax.x);
			ret.y = C_Random::range(aMin.y, aMax.y);
			ret.z = C_Random::range(aMin.z, aMax.z);
			return ret;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length of vector
		inline float length()
		{
			return sqrt((x * x) + (y * y) + (z * z));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return length between 2 vectors
		inline float length(const C_Vector3 aVec)
		{
			return sqrt(pow(aVec.x - x, 2) + pow(aVec.y - y, 2) + pow(aVec.z - z, 2));
		}
		////////////////////////////////////////////////////////////////////////////
		//Return normalize vector
		inline C_Vector3 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z));
			return C_Vector3(x / l, y / l, z / l);
		}
		////////////////////////////////////////////////////////////////////////////
		//Destructor
		inline ~C_Vector3() {}
	};

	typedef C_Vector3 vec3;

}
