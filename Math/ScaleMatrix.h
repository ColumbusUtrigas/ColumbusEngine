#pragma once

#include <Math/Matrix.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class ScaleMatrix : public Matrix
	{
	public:
		inline ScaleMatrix(float Scale) :
			Matrix(Vector4(Scale, 0.0f, 0.0f, 0.0f),
			       Vector4(0.0f, Scale, 0.0f, 0.0f),
			       Vector4(0.0f, 0.0f, Scale, 0.0f),
			       Vector4(0.0f, 0.0f, 0.0f, 1.0f))
		{ }

		inline ScaleMatrix(Vector3 Scale) :
			Matrix(Vector4(Scale.x, 0.0f, 0.0f, 0.0f),
			       Vector4(0.0f, Scale.y, 0.0f, 0.0f),
			       Vector4(0.0f, 0.0f, Scale.z, 0.0f),
			       Vector4(0.0f, 0.0f, 0.0f, 1.0f))
		{ }

		inline static Matrix Make(Vector3 Scale)
		{
			return ScaleMatrix(Scale);
		}

		inline static Matrix Make(float Scale)
		{
			return ScaleMatrix(Scale);
		}
	};

}





