#pragma once

#include <Graphics/Particles/Velocity/ParticleModuleVelocityBase.h>

namespace Columbus
{

	class ParticleModuleVelocity : public ParticleModuleVelocityBase
	{ 
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleVelocity() :
			Min(-1, -1, -1),
			Max(1, 1, 1)
		{ }
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_VELOCITY; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.velocity = Vector3::Random(Min, Max);
		}

		~ParticleModuleVelocity() { }
	};

}













