#pragma once

#include <Math/Vector3.h>

namespace Columbus
{

	class ParticleModuleAcceleration
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleAcceleration() : Min(0), Max(0) {}

		~ParticleModuleAcceleration() {}
	};

}


