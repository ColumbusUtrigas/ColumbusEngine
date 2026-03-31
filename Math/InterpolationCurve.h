#pragma once

#include <algorithm>
#include <vector>

#include <Core/Reflection.h>
#include <Math/Vector2.h>
#include <Math/Vector3.h>
#include <Math/Vector4.h>

namespace Columbus
{
	enum class EInterpolationCurveMode
	{
		Linear,
		Bezier,
		Constant
	};

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

		EInterpolationCurveMode Mode = EInterpolationCurveMode::Linear;
		std::vector<Node> Points;
	public:
		InterpolationCurve() {}

		void AddPoint(const PointType& Point, float Position)
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
			if (Points.empty())
				return PointType{};
			if (Points.size() == 1)
				return Points.front().Value;
			if (Position <= Points.front().Key)
				return Points.front().Value;
			if (Position >= Points.back().Key)
				return Points.back().Value;

			for (size_t Index = 1; Index < Points.size(); Index++)
			{
				if (Position <= Points[Index].Key)
					return InterpolateSegment(Index, Position);
			}

			return Points.back().Value;
		}

		~InterpolationCurve() {}

	private:
		static PointType CubicBezierInterpolate(const PointType& P0, const PointType& P1, const PointType& P2, const PointType& P3, float T)
		{
			const float U = 1.0f - T;
			const float TT = T * T;
			const float UU = U * U;
			const float UUU = UU * U;
			const float TTT = TT * T;

			return P0 * UUU + P1 * (3.0f * UU * T) + P2 * (3.0f * U * TT) + P3 * TTT;
		}

		PointType InterpolateSegment(size_t UpperIndex, float Position) const
		{
			const Node& Previous = Points[UpperIndex - 1];
			const Node& Next = Points[UpperIndex];

			const float SegmentLength = std::max(Next.Key - Previous.Key, 0.0001f);
			const float T = std::clamp((Position - Previous.Key) / SegmentLength, 0.0f, 1.0f);

			switch (Mode)
			{
				case EInterpolationCurveMode::Constant:
					return Previous.Value;
				case EInterpolationCurveMode::Bezier:
				{
					const PointType P0 = UpperIndex > 1 ? Points[UpperIndex - 2].Value : Previous.Value;
					const PointType P1 = Previous.Value;
					const PointType P2 = Next.Value;
					const PointType P3 = UpperIndex + 1 < Points.size() ? Points[UpperIndex + 1].Value : Next.Value;
					const PointType Tangent1 = (P2 - P0) * 0.5f;
					const PointType Tangent2 = (P3 - P1) * 0.5f;
					const PointType Control1 = P1 + Tangent1 / 3.0f;
					const PointType Control2 = P2 - Tangent2 / 3.0f;
					return CubicBezierInterpolate(P1, Control1, Control2, P2, T);
				}
				case EInterpolationCurveMode::Linear:
				default:
					return Previous.Value * (1.0f - T) + Next.Value * T;
			}
		}
	};

	// just definitions of specific types because reflection doesn't support templates
	using InterpolationCurveFloat1 = InterpolationCurve<float>;
	using InterpolationCurveFloat2 = InterpolationCurve<Vector2>;
	using InterpolationCurveFloat3 = InterpolationCurve<Vector3>;
	using InterpolationCurveFloat4 = InterpolationCurve<Vector4>;
}

CREFLECT_DECLARE_ENUM(Columbus::EInterpolationCurveMode, "42E66CD8-FEA6-43CF-81CB-294D883EE41C")

CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat1, 1, "62E535C5-2C95-44C0-9F20-B37F8BC2BA1E")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat2, 1, "B4E4B8D2-A39F-46D1-8FB6-01AF0C922F50")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat3, 2, "4509C7F2-CC24-4282-8233-16B2B08A958C")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat4, 2, "133CFE41-FD53-411E-B906-6078C3D297A8")

CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat1::Node, 1, "4E709E8B-A252-4F40-B465-D3D791748A50")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat2::Node, 1, "5206D42A-1E83-4F25-A572-7A50F5B3C0A2")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat3::Node, 1, "12777AEA-1253-43AD-81FE-1B166ABAE0F9")
CREFLECT_DECLARE_STRUCT(Columbus::InterpolationCurveFloat4::Node, 1, "8336FEB5-2370-4996-8662-C71E4CCB0161")
