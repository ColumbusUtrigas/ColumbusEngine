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

		inline C_Vector2() {}
		inline C_Vector2(const float aX, const float aY) : x((float)aX), y((float)aY) {}
		inline C_Vector2(const glm::vec2 aVec) : x(aVec.x), y(aVec.y) {}

		inline void fromGLM(const glm::vec2 aVec)
		{
			x = aVec.x;
			y = aVec.y;
		}

		inline glm::vec2 toGLM()
		{
			return glm::vec2(x, y);
		}

		inline C_Vector2& operator=(C_Vector2 aOther)
		{
			x = aOther.x;
			y = aOther.y;
			return *this;
		}

		inline C_Vector2 operator+(const C_Vector2 aOther)
		{
			x += aOther.x;
			y += aOther.y;
			return *this;
		}

		inline C_Vector2 operator-(const C_Vector2 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
			return *this;
		}

		inline C_Vector2 operator*(const C_Vector2 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
			return *this;
		}


		inline C_Vector2 operator*(const float aOther)
		{
			x *= aOther;
			y *= aOther;
			return *this;
		}

		inline C_Vector2 operator/(const C_Vector2 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
			return *this;
		}

		inline bool operator==(C_Vector2 aOther)
		{
			return (x == aOther.x && y == aOther.y);
		}

		inline bool operator!=(C_Vector2 aOther)
		{
			return (x != aOther.x && y != aOther.y);
		}

		inline C_Vector2 operator+=(C_Vector2 aOther)
		{
			x += aOther.x;
			y += aOther.y;
		}

		inline C_Vector2 operator-=(C_Vector2 aOther)
		{
			x -= aOther.x;
			y -= aOther.y;
		}

		inline C_Vector2 operator*=(C_Vector2 aOther)
		{
			x *= aOther.x;
			y *= aOther.y;
		}

		inline C_Vector2 operator/=(C_Vector2 aOther)
		{
			x /= aOther.x;
			y /= aOther.y;
		}

		inline ~C_Vector2() {}
	};

	typedef C_Vector2 vec2;

}
