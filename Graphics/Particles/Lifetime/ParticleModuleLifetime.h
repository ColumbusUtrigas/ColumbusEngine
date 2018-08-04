#pragma once

#include <Graphics/Particles/Lifetime/ParticleModuleLifetimeBase.h>

namespace Columbus
{

	class ParticleModuleLifetime : public ParticleModuleLifetimeBase
	{
	public:
		float Min;
		float Max;

		ParticleModuleLifetime() : Min(1.0f), Max(1.0f) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Lifetime; }
		/*
		* Random value between Min and Max
		*/
		float GetLifetime() const
		{
			return Random::Range(Min, Max);
		}
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.TTL = Random::Range(Min, Max);
		}

		~ParticleModuleLifetime() override {}
	};

}







