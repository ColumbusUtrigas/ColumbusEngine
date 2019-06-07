#pragma once

#include <map>
#include <vector>
#include <algorithm>

namespace Columbus
{

	template <typename PointType>
	class InterpolationCurve
	{
	public:
		std::vector<std::pair<float, PointType>> Points;
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
				{ return a.first < b.first; });
		}
		/*
		* Interpolate curve
		* @param float Position: Position of X on curve
		*/		
		PointType Interpolate(float Position) const
		{
			PointType Result;
			PointType Down, Up;

			if (Points.size() >= 2)
			{
				float DownPos = 0.0f, UpPos = 0.0f;
				float Percent = 0.0f;

				for (auto& Point : Points)
				{
					DownPos = UpPos;
					Down = Up;
					UpPos = Point.first;
					Up = Point.second;

					if (Position < Point.first)
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
				return Points.begin()->second;
			}

			return Result;
		}

		~InterpolationCurve() {}
	};

}


