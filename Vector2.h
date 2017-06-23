#pragma once

#include <cmath>
#include <glm/glm.hpp>

namespace C
{

	struct C_Vector2
	{
		float x;
		float y;

		C_Vector2()
		{
			x = 0;
			y = 0;
		}

		inline C_Vector2(const float aX, const float aY)
		{
			x = (float)aX;
			y = (float)aY;
		}

		inline C_Vector2 operator=(C_Vector2 other)
		{
			x = other.x;
			y = other.y;
			return *this;
		}

		inline friend C_Vector2& operator+(C_Vector2& left, const C_Vector2& right)
		{
			left.x += right.x;
			left.y += right.y;
			return left;
		}

		inline friend C_Vector2& operator-(C_Vector2& left, const C_Vector2& right)
		{
			left.x -= right.x;
			left.y -= right.y;
			return left;
		}

		inline friend C_Vector2& operator*(C_Vector2& left, const C_Vector2& right)
		{
			left.x *= right.x;
			left.y *= right.y;
			return left;
		}

		inline friend C_Vector2& operator*(C_Vector2& left, const float right)
		{
			left.x *= right;
			left.y *= right;
			return left;
		}

		inline friend C_Vector2& operator/(C_Vector2& left, const C_Vector2& right)
		{
			left.x /= right.x;
			left.y /= right.y;
			return left;
		}

		inline bool operator==(const C_Vector2& other)
		{
			return x == other.x && y == other.y;
		}

		inline bool operator!=(const C_Vector2& other)
		{
			return x != other.x && y != other.y;
		}

		inline C_Vector2 operator+=(const C_Vector2& other)
		{
			x += other.x;
			y += other.y;
		}

		inline C_Vector2 operator-=(const C_Vector2& other)
		{
			x -= other.x;
			y -= other.y;
		}

		inline C_Vector2 operator*=(const C_Vector2& other)
		{
			x *= other.x;
			y *= other.y;
		}

		inline C_Vector2 operator/=(const C_Vector2& other)
		{
			x /= other.x;
			y /= other.y;
		}

		inline void normalize()
		{
			float norm = sqrt(x * x + y * y);
			if(x != 0)
				x = x / norm;
			if(y != 0)
				y = y / norm;
		}

		inline glm::vec2 toGLM()
		{
			return glm::vec2(x, y);
		}

		inline void fromGLM(glm::vec2 aVec)
		{
			x = aVec.x;
			y = aVec.y;
		}
	};

	typedef C_Vector2 vec2;

}
