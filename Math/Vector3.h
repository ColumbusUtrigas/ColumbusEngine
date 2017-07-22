/************************************************
*              		  Vector3.h                   *
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
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator -
		inline C_Vector3 operator-(const C_Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector3 operator*(const C_Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator *
		inline C_Vector3 operator*(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Opeator /
		inline C_Vector3 operator/(const C_Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator ==
		inline bool operator==(C_Vector3 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(C_Vector3 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector3 operator+=(C_Vector3 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector3 operator-=(C_Vector3 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector3 operator*=(C_Vector3 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector3 operator/=(C_Vector3 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector3 random(C_Vector3 aMin, C_Vector3 aMax)
		{
			C_Vector3 ret;
			ret.x = aMin.x + (float)(rand()) / ((float)(RAND_MAX / (aMax.x - aMin.x)));
			ret.y = aMin.y + (float)(rand()) / ((float)(RAND_MAX / (aMax.y - aMin.y)));
			ret.z = aMin.z + (float)(rand()) / ((float)(RAND_MAX / (aMax.z - aMin.z)));
			return ret;
		}
		//Return normalize vector
		inline C_Vector3 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z));
			x /= l;
			y /= l;
			z /= l;
		}
		//Destructor
		inline ~C_Vector3() {}
	};

	typedef C_Vector3 vec3;

}
