#pragma once

#include <Graphics/Particles/Acceleration/ParticleModuleAccelerationBase.h>

namespace Columbus
{

	class ParticleModuleAcceleration : public ParticleModuleAccelerationBase
	{
	public:
		Vector3 Min;
		Vector3 Max;

		ParticleModuleAcceleration() :
			Min(Vector3(0, 0, 0)),
			Max(Vector3(0, 0, 0)) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_ACCELERATION; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.accel = Vector3::random(Min, Max);
		}

		~ParticleModuleAcceleration() override {}
	};

}





