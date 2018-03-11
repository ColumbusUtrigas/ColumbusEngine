#pragma once

#include <Graphics/Particles/Velocity/ParticleModuleVelocityBase.h>

namespace Columbus
{

	class ParticleModuleVelocity : public ParticleModuleVelocityBase
	{ 
	public:
		Vector3 Min = Vector3(-1, -1, -1);
		Vector3 Max = Vector3(1, 1, 1);

		ParticleModuleVelocity() {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_VELOCITY; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.velocity = Vector3::random(Min, Max);
		}

		~ParticleModuleVelocity() {}
	};

}













