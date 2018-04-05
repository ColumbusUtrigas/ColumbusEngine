#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	class Quaternion
	{
	public:
		float X;
		float Y;
		float Z;
		float W;
	public:
		Quaternion() :
			X(0.0f),
			Y(0.0f),
			Z(0.0f),
			W(0.0f)
		{}

		Quaternion(float InX, float InY, float InZ, float InW) :
		 	X(InX),
		 	Y(InY),
		 	Z(InZ),
		 	W(InW)
		{}

		 Quaternion(Vector3 Axis, float Angle) {}

		/*
		* Convert Quaternion into Euler angle (in degrees)
		*/
		Vector3 Euler() const;

		~Quaternion() {}
	};

}








