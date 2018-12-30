#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleAcceleration : public ParticleModule
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleAcceleration() :
			Min(0, 0, 0),
			Max(0, 0, 0)
		{ }
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Acceleration; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.accel = Vector3::Random(Min, Max);
		}

		~ParticleModuleAcceleration() override { }
	};

}





