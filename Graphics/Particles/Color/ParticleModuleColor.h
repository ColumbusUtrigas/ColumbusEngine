#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleColor : public ParticleModule
	{
	public:
		Vector4 Min;
		Vector4 Max;
	public:
		ParticleModuleColor() :
			Min(Vector4(1, 1, 1, 1)),
			Max(Vector4(1, 1, 1, 1)) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Color; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.Color = Vector4::Random(Min, Max);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override {}

		~ParticleModuleColor() override {}
	};

}





