#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	class AudioListener
	{
	public:
		Vector3 Position;
		Vector3 Right;
		Vector3 Up;
		Vector3 Forward;
	public:
		AudioListener() :
			Right(1, 0, 0),
			Up(0, 1, 0),
			Forward(0, 0, -1) {}

		~AudioListener() {}
	};

}







