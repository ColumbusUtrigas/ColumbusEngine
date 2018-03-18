#pragma once

#include <Graphics/Particles/Color/ParticleModuleColorBase.h>

namespace Columbus
{

	class ParticleModuleColorOverLife : public ParticleModuleColorBase
	{
	public:
		Vector4 MinStart;
		Vector4 MaxStart;
		Vector4 MinFinal;
		Vector4 MaxFinal;

		ParticleModuleColorOverLife() :
			MinStart(Vector4(1, 1, 1, 1)),
			MaxStart(Vector4(1, 1, 1, 1)),
			MinFinal(Vector4(1, 1, 1, 1)),
			MaxFinal(Vector4(1, 1, 1, 1)) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_COLOR; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.startColor = Vector4::random(MinStart, MaxStart);
			aParticle.finalColor = Vector4::random(MinFinal, MaxFinal);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override
		{
			aParticle.color = aParticle.startColor * (1 - aParticle.percent) + aParticle.finalColor * aParticle.percent;
		}

		~ParticleModuleColorOverLife() override {}
	};

}





