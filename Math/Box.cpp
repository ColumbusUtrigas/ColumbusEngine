#include "Box.h"
#include <cmath>

namespace Columbus
{
	bool Box::IsValid() const
	{
		return std::isfinite(Min.X) && std::isfinite(Min.Y) && std::isfinite(Min.Z) &&
		       std::isfinite(Max.X) && std::isfinite(Max.Y) && std::isfinite(Max.Z) &&
		       Min.X <= Max.X && Min.Y <= Max.Y && Min.Z <= Max.Z;
	}

	Box Box::CalcTransformedBox(const Matrix& Transformation) const
	{
		if (!IsValid())
		{
			return *this;
		}

		const Vector3 Center = (Transformation * Vector4(CalcCenter(), 1)).XYZ();
		const Vector3 Extent = CalcSize() * 0.5f;
		const Vector3 TransformedExtent(
		    fabsf(Transformation.M[0][0]) * Extent.X + fabsf(Transformation.M[0][1]) * Extent.Y + fabsf(Transformation.M[0][2]) * Extent.Z,
		    fabsf(Transformation.M[1][0]) * Extent.X + fabsf(Transformation.M[1][1]) * Extent.Y + fabsf(Transformation.M[1][2]) * Extent.Z,
		    fabsf(Transformation.M[2][0]) * Extent.X + fabsf(Transformation.M[2][1]) * Extent.Y + fabsf(Transformation.M[2][2]) * Extent.Z);
		return Box(Center - TransformedExtent, Center + TransformedExtent);
	}

	Box Box::CalcDeformedBox(std::span<const Box> MorphBounds, std::span<const float> MorphWeights, std::span<const Matrix> SkinMatrices) const
	{
		if (!IsValid())
		{
			return *this;
		}

		if (MorphWeights.size() > MorphBounds.size())
		{
			return Box(Vector3(1), Vector3(-1));
		}

		Box Bounds = *this;

		for (size_t i = 0; i < MorphWeights.size(); ++i)
		{
			const float Weight = MorphWeights[i];

			if (!std::isfinite(Weight) || !MorphBounds[i].IsValid())
			{
				return Box(Vector3(1), Vector3(-1));
			}

			if (fabsf(Weight) <= 0.000001f)
			{
				continue; // Match DeformMesh.hlsl's inactive morph threshold.
			}

			const Vector3 MinDelta = MorphBounds[i].Min * Weight;
			const Vector3 MaxDelta = MorphBounds[i].Max * Weight;
			Bounds.Min += Vector3::Min(MinDelta, MaxDelta);
			Bounds.Max += Vector3::Max(MinDelta, MaxDelta);
		}

		// Every normalized positive blend lies inside the union of the transformed
		// bounds. Include the original bounds for vertices without skin influences.
		Box Result = Bounds;

		for (const Matrix& Skin : SkinMatrices)
		{
			const Box Transformed = Bounds.CalcTransformedBox(Skin);

			if (!Transformed.IsValid())
			{
				return Transformed;
			}

			Result.Min = Vector3::Min(Result.Min, Transformed.Min);
			Result.Max = Vector3::Max(Result.Max, Transformed.Max);
		}

		return Result;
	}

}
