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
	public:
		ParticleModuleSizeOverLife() : 
			MinStart(1, 1, 1),
			MaxStart(1, 1, 1),
			MinFinal(1, 1, 1),
			MaxFinal(1, 1, 1)
		{ }
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_SIZE; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			aParticle.startSize = Vector3::Random(MinStart, MaxStart);
			aParticle.finalSize = Vector3::Random(MinFinal, MaxFinal);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override
		{
			aParticle.size = aParticle.startSize * (1 - aParticle.percent) + aParticle.finalSize * aParticle.percent;
		}

		~ParticleModuleSizeOverLife() override { }
	};

}




