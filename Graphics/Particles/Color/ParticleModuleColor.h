#pragma once

#include <Graphics/Particles/Color/ParticleModuleColorBase.h>

namespace Columbus
{

	class ParticleModuleColor : public ParticleModuleColorBase
	{
	public:
		Vector4 Min;
		Vector4 Max;

		ParticleModuleColor() :
			Min(Vector4(1, 1, 1, 1)),
			Max(Vector4(1, 1, 1, 1)) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_COLOR; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.color = Vector4::random(Min, Max);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override {}

		~ParticleModuleColor() override {}
	};

}





