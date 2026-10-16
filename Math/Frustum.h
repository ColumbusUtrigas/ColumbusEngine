#pragma once

#include <Math/Matrix.h>
#include <Math/Vector4.h>
#include <Math/Box.h>
#include <algorithm>
#include <cmath>
#include <span>
#include "Geometry.h"

namespace Columbus
{

	class Frustum
	{
	private:
		Vector4 Planes[6];
	public:
		enum class DepthRange
		{
			MinusOneToOne,
			ZeroToOne
		};

		Frustum(const Matrix& ViewProjection, DepthRange ClipDepthRange)
		{
			SetProjection(ViewProjection, ClipDepthRange);
		}

		void SetProjection(const Matrix& ViewProjection, DepthRange ClipDepthRange)
		{
			const Vector4 X = ViewProjection.GetRow(0);
			const Vector4 Y = ViewProjection.GetRow(1);
			const Vector4 Z = ViewProjection.GetRow(2);
			const Vector4 W = ViewProjection.GetRow(3);

			// Column vectors and row-major matrices. Zero-to-one also covers reverse Z.
			Planes[0] = W + X;
			Planes[1] = W - X;
			Planes[2] = W + Y;
			Planes[3] = W - Y;
			Planes[4] = ClipDepthRange == DepthRange::ZeroToOne ? Z : W + Z;
			Planes[5] = W - Z;

			for (Vector4& Plane : Planes)
			{
				const float Length = Plane.XYZ().Length();
				Plane              = Length > 1e-8f ? Plane / Length : Vector4(0);
			}
		}

		// Conservative AABB test with scale-aware tolerance. Unknown bounds remain visible.
		bool Intersects(const Box& Bounds) const
		{
			if (!Bounds.IsValid())
			{
				return true;
			}

			const Vector3 Center = Bounds.CalcCenter();
			const Vector3 Extent = Bounds.CalcSize() * 0.5f;

			const float Tolerance = 0.0001f * std::max({
				1.0f,
				fabsf(Center.X) + Extent.X,
				fabsf(Center.Y) + Extent.Y,
				fabsf(Center.Z) + Extent.Z
			});

			for (const Vector4& Plane : Planes)
			{
				const float Distance = Vector3::Dot(Plane.XYZ(), Center) + Plane.W;
				const float Radius   = fabsf(Plane.X) * Extent.X + fabsf(Plane.Y) * Extent.Y + fabsf(Plane.Z) * Extent.Z;

				if (Distance + Radius < -Tolerance)
				{
					return false;
				}
			}

			return true;
		}

		bool Intersects(const Geometry::Sphere& Bounds) const
		{
			const Vector3 Center = Bounds.Center;
			const float Radius = Bounds.Radius;

			if (!std::isfinite(Center.X) || !std::isfinite(Center.Y) || !std::isfinite(Center.Z) || !std::isfinite(Radius) || Radius < 0)
			{
				return true;
			}

			const float Tolerance = 0.0001f * std::max({ 1.0f, fabsf(Center.X), fabsf(Center.Y), fabsf(Center.Z), Radius });

			for (const Vector4& Plane : Planes)
			{
				if (Vector3::Dot(Plane.XYZ(), Center) + Plane.W + Radius < -Tolerance)
				{
					return false;
				}
			}

			return true;
		}

		// Conservative convex-hull test. Margin expands each plane in world units.
		bool Intersects(std::span<const Vector3> Points, float Margin = 0) const
		{
			if (Points.empty())
			{
				return true;
			}

			for (const Vector4& Plane : Planes)
			{
				bool AllOutside = true;

				for (const Vector3& Point : Points)
				{
					const float Tolerance = 0.0001f * std::max({ 1.0f, fabsf(Point.X), fabsf(Point.Y), fabsf(Point.Z) });
					const float Distance = Vector3::Dot(Plane.XYZ(), Point) + Plane.W;

					if (!std::isfinite(Distance) || Distance >= -Margin - Tolerance)
					{
						AllOutside = false;
						break;
					}
				}

				if (AllOutside)
				{
					return false;
				}
			}

			return true;
		}

		Frustum() {}
		Frustum(const Matrix& ViewProjection) { SetProjection(ViewProjection); }

		void SetProjection(const Matrix& ViewProjection)
		{
			const auto M = &ViewProjection.M[0][0];

			const auto ExtractPlane = [&](Vector4& Plane, int Row)->void
			{
				int Scale = Row < 0 ? -1 : 1;
				Row = Math::Abs(Row) - 1;

				Plane.X = M[3]  + Scale * M[Row];
				Plane.Y = M[7]  + Scale * M[Row + 4];
				Plane.Z = M[11] + Scale * M[Row + 8];
				Plane.W = M[15] + Scale * M[Row + 12];

				float Length = Math::Sqrt(Plane.X * Plane.X + Plane.Y * Plane.Y + Plane.Z * Plane.Z);
				Plane /= Length;
			};

			ExtractPlane(Planes[0], +1);
			ExtractPlane(Planes[1], -1);
			ExtractPlane(Planes[2], +2);
			ExtractPlane(Planes[3], -2);
			ExtractPlane(Planes[4], +3);
			ExtractPlane(Planes[5], -3);

			//Planes[0] = Vector4(M[0][3] - M[0][1], M[1][3] - M[1][1], M[2][3] - M[2][1], -M[3][3] + M[3][1]); //Top
			//Planes[1] = Vector4(M[0][3] + M[0][1], M[1][3] + M[1][1], M[2][3] + M[2][1], -M[3][3] - M[3][1]); //Bottom
			//Planes[2] = Vector4(M[0][3] + M[0][0], M[1][3] + M[1][0], M[2][3] + M[2][0], -M[3][3] - M[3][0]); //Left
			//Planes[3] = Vector4(M[0][3] - M[0][0], M[1][3] - M[1][0], M[2][3] - M[2][0], -M[3][3] + M[3][0]); //Right
			//Planes[4] = Vector4(M[0][3] + M[0][2], M[1][3] + M[1][2], M[2][3] + M[2][2], -M[3][3] - M[3][2]); //Near
			//Planes[5] = Vector4(M[0][3] - M[0][2], M[1][3] - M[1][2], M[2][3] - M[2][2], -M[3][3] + M[3][2]); //Far

			/*for (int i = 0; i < 6; i++)
			{
				Planes[i] *= 1.0 / Math::Sqrt(Math::Pow(Planes[i].X, 2) + Math::Pow(Planes[i].Y, 2) + Math::Pow(Planes[i].Z, 2));
			}*/
		}

		bool Check(const Vector3& Point)
		{
			for (int i = 0; i < 6; i++)
			{
				float d = Planes[i].X * Point.X + Planes[i].Y * Point.Y + Planes[i].Z * Point.Z + Planes[i].W;

				if (d < 0)
				{
					return false;
				}
			}

			return true;
		}

		bool Check(const Box& AABB)
		{
			for (int i = 0; i < 6; i++)
			{
				float d = Math::Max(AABB.Min.X * Planes[i].X, AABB.Max.X * Planes[i].X)
				        + Math::Max(AABB.Min.Y * Planes[i].Y, AABB.Max.Y * Planes[i].Y)
				        + Math::Max(AABB.Min.Z * Planes[i].Z, AABB.Max.Z * Planes[i].Z)
				        + Planes[i].W;
				
				if (d < 0) return false;
			}

			return true;
		}

		/*bool Check(const Box& BoundingBox, const Matrix& ModelMatrix)
		{
			return true;
		}*/

		~Frustum() {}
	};

}


