#pragma once

#include <map>

#include <iostream>

namespace Columbus
{

	template <typename PointType>
	class InterpolationCurve
	{
	private:
		std::map<float, PointType> Points;
	public:
		InterpolationCurve() {}

		void AddPoint(PointType Point, float Position)
		{
			if (Position >= 0.0f && Position <= 1.0f)
			{
				Points[Position] = Point;
			}
		}
		/*
		* Interpolate curve
		* @param float Position: Position of X on curve
		* Position must be >= 0 and <= 1
		*/		
		PointType Interpolate(float Position)
		{
			PointType Result;

			if (Points.size() >= 2)
			{
				if (Position >= 0.0f && Position <= 1.0f)
				{
					PointType Down, Up;
					float DownPos = 0.0f, UpPos = 0.0f;
					float Percent = 0.0f;

					for (auto& Point : Points)
					{
						if (Position < Point.first)
						{
							DownPos = UpPos;
							Down = Up;
							UpPos = Point.first;
							Up = Point.second;
							break;
						}

						DownPos = UpPos;
						Down = Up;
						UpPos = Point.first;
						Up = Point.second;
					}

					float Length = UpPos - DownPos;

					if (Length > 0.00001)
					{
						Percent = (Position - DownPos) / Length;
					}

					Result = Down * (1 - Percent) + Up * Percent;
				}
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


















