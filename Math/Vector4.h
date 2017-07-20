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

		inline C_Vector4() {}
		inline C_Vector4(float aX, float aY, float aZ, float aW) : x(aX), y(aY), z(aZ), w(aW) {}
		inline C_Vector4(glm::vec4 aVec) : x(aVec.x), y(aVec.y), z(aVec.z), w(aVec.w) {}

		inline void fromGLM(const glm::vec4 aVec)
		{
			x = aVec.x;
			y = aVec.y;
			z = aVec.z;
			w = aVec.w;
		}

		inline glm::vec4 toGLM()
		{
			return glm::vec4(x, y, z, w);
		}

		inline C_Vector4& operator=(C_Vector4 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			z = aOther.z;
			w = aOther.w;
			return *this;
		}

		inline C_Vector4 operator+(const C_Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}

		inline C_Vector4 operator-(const C_Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}

		inline C_Vector4 operator*(const C_Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}

		inline C_Vector4 operator*(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			z *= aOther;
			w *= aOther;
			return *this;
		}

		inline C_Vector4 operator/(const C_Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}

		inline bool operator==(C_Vector4 aOther)
		{
			return (x == aOther.x && y == aOther.y && z == aOther.z && w == aOther.w);
		}

		inline bool operator!=(C_Vector4 aOther)
		{
			return (x != aOther.x && y != aOther.y && z != aOther.z && w != aOther.w);
		}

		inline C_Vector4 operator+=(C_Vector4 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			z += aOther.z;
			w += aOther.w;
			return *this;
		}

		inline C_Vector4 operator-=(C_Vector4 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			z -= aOther.z;
			w -= aOther.w;
			return *this;
		}

		inline C_Vector4 operator*=(C_Vector4 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			z *= aOther.z;
			w *= aOther.w;
			return *this;
		}

		inline C_Vector4 operator/=(C_Vector4 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			z /= aOther.z;
			w /= aOther.w;
			return *this;
		}

		inline static C_Vector4 random(C_Vector4 aMin, C_Vector4 aMax)
		{
			C_Vector4 ret;
			ret.x = aMin.x + (float)(rand()) / ((float)(RAND_MAX / (aMax.x - aMin.x)));
			ret.y = aMin.y + (float)(rand()) / ((float)(RAND_MAX / (aMax.y - aMin.y)));
			ret.z = aMin.z + (float)(rand()) / ((float)(RAND_MAX / (aMax.z - aMin.z)));
			ret.w = aMin.w + (float)(rand()) / ((float)(RAND_MAX / (aMax.w - aMin.w)));
			return ret;
		}

		inline C_Vector4 normalize()
		{
			float l = sqrt((x * x) + (y * y) + (z * z) + (w * w));
			x /= l;
			y /= l;
			z /= l;
			w /= l;
		}

		inline ~C_Vector4() {}
	};

	typedef C_Vector4 vec4;

}
