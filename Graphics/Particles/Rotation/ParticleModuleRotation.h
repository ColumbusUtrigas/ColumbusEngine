#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleRotation : public ParticleModule
	{
	public:
		float Min;
		float Max;
		float MinVelocity;
		float MaxVelocity;
	public:
		ParticleModuleRotation() : Min(0.0f), Max(0.0f), MinVelocity(0.0f), MaxVelocity(0.0f) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Rotation; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.rotation = Random::Range(Min, Max);
			OutParticle.rotationSpeed = Random::Range(MinVelocity, MaxVelocity);
		}

		void Spawn(const ParticleContainer& Container, size_t Spawn) const;
		void Update(ParticleContainer& Container) const;

		~ParticleModuleRotation() override {}
	};

}


