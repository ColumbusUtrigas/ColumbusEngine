#include "Box.h"

#include <limits.h>

namespace Columbus
{

	Box Box::CalcTransformedBox(const Matrix& Transformation) const
	{
		Vector3 Center = CalcCenter();
		Vector3 HalfSize = CalcSize() / 2;

		// 1. find vertices of this box
		Vector4 Vertices[8]{
			Vector4(Center + HalfSize * Vector3(+1, +1, +1), 1), // +++
			Vector4(Center + HalfSize * Vector3(-1, +1, +1), 1), // -++
			Vector4(Center + HalfSize * Vector3(+1, -1, +1), 1), // +-+
			Vector4(Center + HalfSize * Vector3(-1, -1, +1), 1), // --+

			Vector4(Center + HalfSize * Vector3(+1, +1, -1), 1), // ++-
			Vector4(Center + HalfSize * Vector3(-1, +1, -1), 1), // -+-
			Vector4(Center + HalfSize * Vector3(+1, -1, -1), 1), // +--
			Vector4(Center + HalfSize * Vector3(-1, -1, -1), 1), // ---
		};

		Box Result;
		Result.Min = Vector3(FLT_MAX);
		Result.Max = Vector3(FLT_MIN);

		for (int i = 0; i < 8; i++)
		{
			// 2. transform vertices by matrix
			Vertices[i] = Transformation * Vertices[i];

			// 3. find min/max of vertices
			Result.Min.X = Math::Min(Result.Min.X, Vertices[i].X);
			Result.Min.Y = Math::Min(Result.Min.Y, Vertices[i].Y);
			Result.Min.Z = Math::Min(Result.Min.Z, Vertices[i].Z);

			Result.Max.X = Math::Max(Result.Max.X, Vertices[i].X);
			Result.Max.Y = Math::Max(Result.Max.Y, Vertices[i].Y);
			Result.Max.Z = Math::Max(Result.Max.Z, Vertices[i].Z);
		}

		return Result;
	}

}
