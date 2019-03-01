#pragma once

#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	class ParticleModuleVelocity : public ParticleModule
	{ 
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleVelocity() : Min(-1), Max(1) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Velocity; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.velocity = Vector3::Random(Min, Max);
		}

		void Spawn(const ParticleContainer& Container, size_t Spawn)
		{
			for (size_t i = Container.Count; i < Container.Count + Spawn; i++)
			{
				Container.Velocities[i] = Vector3::Random(Min, Max);
			}
		}

		~ParticleModuleVelocity() {}
	};

}


