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
		virtual ParticleModuleType GetType() const { return E_PARTICLE_MODULE_ACCELERATION; }
		/*
		* Set parameter for particle
		*/
		virtual void Spawn(Particle& aParticle)
		{
			aParticle.accel = Vector3::random(Min, Max);
		}

		~ParticleModuleAcceleration() override {}
	};

}





