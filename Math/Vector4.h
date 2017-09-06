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

namespace C
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
		//Operator ==
		inline bool operator==(C_Vector4 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator !=
		inline bool operator!=(C_Vector4 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z && w != aOther.w);
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator +=
		inline C_Vector4 operator+=(C_Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator -=
		inline C_Vector4 operator-=(C_Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator *=
		inline C_Vector4 operator*=(C_Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Operator /=
		inline C_Vector4 operator/=(C_Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}
		////////////////////////////////////////////////////////////////////////////
		//Return random from two vectors
		inline static C_Vector4 random(C_Vector4 aMin, C_Vector4 aMax)
		{
			C_Vector4 ret;
			ret.x = aMin.x + (float)(rand()) / ((float)(RAND_MAX / (aMax.x - aMin.x)));
			ret.y = aMin.y + (float)(rand()) / ((float)(RAND_MAX / (aMax.y - aMin.y)));
			ret.z = aMin.z + (float)(rand()) / ((float)(RAND_MAX / (aMax.z - aMin.z)));
			ret.w = aMin.w + (float)(rand()) / ((float)(RAND_MAX / (aMax.w - aMin.w)));
			return ret;
		}
		//Return normalize vector
		inline C_Vector4 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z) + (w * w));
			return C_Vector4(x / l, y / l, z / l, w / l);
		}
		//Destructor
		inline ~C_Vector4() {}
	};

	typedef C_Vector4 vec4;

}
