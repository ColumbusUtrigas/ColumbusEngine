#pragma once

#include <Math/Matrix.h>
#include <Math/Vector3.h>
#include <Core/Types.h>

namespace Columbus
{

	class TranslationMatrix : public Matrix
	{
	public:
		inline TranslationMatrix(Vector3 Translation) :
			Matrix(Vector4(1.0f, 0.0f, 0.0f, Translation.X),
			       Vector4(0.0f, 1.0f, 0.0f, Translation.Y),
			       Vector4(0.0f, 0.0f, 1.0f, Translation.Z),
			       Vector4(0.0f, 0.0f, 0.0f, 1.0f))
		{
		}

		inline static Matrix Make(Vector3 Translation)
		{
			return TranslationMatrix(Translation);
		}
	};

}






