#pragma once

#include <Graphics/Particles/Rotation/ParticleModuleRotationBase.h>

namespace Columbus
{

	class ParticleModuleRotation : public ParticleModuleRotationBase
	{
	public:
		float Min;
		float Max;
		float MinVelocity;
		float MaxVelocity;

		ParticleModuleRotation() :
			Min(0.0f),
			Max(0.0f),
			MinVelocity(0.0f),
			MaxVelocity(0.0f) {}
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

		~ParticleModuleRotation() override {}
	};

}




