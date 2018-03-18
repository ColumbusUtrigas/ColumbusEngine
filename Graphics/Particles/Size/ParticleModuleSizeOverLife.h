#pragma once

#include <Graphics/Particles/Size/ParticleModuleSizeBase.h>

namespace Columbus
{

	class ParticleModuleSizeOverLife : public ParticleModuleSizeBase
	{
	public:
		Vector3 MinStart;
		Vector3 MaxStart;
		Vector3 MinFinal;
		Vector3 MaxFinal;

		ParticleModuleSizeOverLife() : 
			MinStart(Vector3(1, 1, 1)),
			MaxStart(Vector3(1, 1, 1)),
			MinFinal(Vector3(1, 1, 1)),
			MaxFinal(Vector3(1, 1, 1)) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_SIZE; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.startSize = Vector3::random(MinStart, MaxStart);
			aParticle.finalSize = Vector3::random(MinFinal, MaxFinal);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override
		{
			aParticle.size = aParticle.startSize * (1 - aParticle.percent) + aParticle.finalSize * aParticle.percent;
		}

		~ParticleModuleSizeOverLife() override {}
	};

}




