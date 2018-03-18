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
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_ROTATION; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.rotation = Random::range(Min, Max);
			aParticle.rotationSpeed = Random::range(MinVelocity, MaxVelocity);
		}

		~ParticleModuleRotation() override {}
	};

}




