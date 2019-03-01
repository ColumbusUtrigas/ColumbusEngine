#pragma once

#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	class ParticleModuleLifetime : public ParticleModule
	{
	public:
		float Min;
		float Max;
	public:
		ParticleModuleLifetime() : Min(1.0f), Max(1.0f) {}
		
		Type GetType() const override { return Type::Lifetime; }
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.TTL = Random::Range(Min, Max);
		}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const;

		~ParticleModuleLifetime() override {}
	};

}


