#pragma once

#include <algorithm>
#include <vector>

#include <Core/Reflection.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{

	template <typename PointType>
	class InterpolationCurve
	{
	public:
		struct Node
		{
			float Key = 0.0f;
			PointType Value = {};

			Node() {}
			Node(float Key, const PointType& Value) :
				Key(Key), Value(Value) {}
		};

		std::vector<Node> Points;
	public:
		InterpolationCurve() {}

		void AddPoint(PointType Point, float Position)
		{
			Points.emplace_back(Position, Point);
			Sort();
		}

		void RemovePoint(size_t Point)
		{
			Points.erase(Points.begin() + Point);
		}

		void Sort()
		{
			std::sort(Points.begin(), Points.end(), [](const auto& a, const auto& b)
				{ return a.Key < b.Key; });
		}
		/*
		* Interpolate curve
		* @param float Position: Position of X on curve, range is between min and max positions
		*/		
		PointType Interpolate(float Position) const
		{
			PointType Result{};
			PointType Down{}, Up{};

			if (Points.size() >= 2)
			{
				float DownPos = 0.0f, UpPos = 0.0f;
				float Percent = 0.0f;

				for (auto& Point : Points)
				{
					DownPos = UpPos;
					Down = Up;
					UpPos = Point.Key;
					Up = Point.Value;

					if (Position < Point.Key)
					{
						break;
					}
				}

				float Length = UpPos - DownPos;

				if (Length > 0.00001)
				{
					Percent = (Position - DownPos) / Length;
				}

				Result = Down * (1 - Percent) + Up * Percent;
			}
			else if(Points.size() == 1)
			{
				return Points.begin()->Value;
			}

			return Result;
		}

		~InterpolationCurve() {}
	};

	// just definitions of specific types because reflection doesn't support templates
	using InterpolationCurveFloat3 = InterpolationCurve<Vector3>;
	using InterpolationCurveFloat4 = InterpolationCurve<Vector4>;
}

CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat3, 1, "4509C7F2-CC24-4282-8233-16B2B08A958C")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat4, 1, "133CFE41-FD53-411E-B906-6078C3D297A8")

CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat3::Node, 1, "12777AEA-1253-43AD-81FE-1B166ABAE0F9")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat4::Node, 1, "8336FEB5-2370-4996-8662-C71E4CCB0161")
