#pragma once

#include <Graphics/Particles/ParticleModule.h>
#include <System/Random.h>

namespace Columbus
{

	class ParticleModuleLifetime : public ParticleModule
	{
	public:
		float Min;
		float Max;

		ParticleModuleLifetime() : Min(1.0f), Max(1.0f) {}
		/*
		* For determening module type
		*/
		virtual ParticleModuleType GetType() const { return E_PARTICLE_MODULE_LIFETIME; }
		/*
		* Random value between Min and Max
		*/
		float GetLifetime() const
		{
			return Random::range(Min, Max);
		}
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.TTL = Random::range(Min, Max);
		}

		~ParticleModuleLifetime() {}
	};

}







