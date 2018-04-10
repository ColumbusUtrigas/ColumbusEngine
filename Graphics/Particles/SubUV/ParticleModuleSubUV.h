#pragma once

#include <Graphics/Particles/SubUV/ParticleModuleSubUVBase.h>

namespace Columbus
{

	class ParticleModuleSubUV : public ParticleModuleSubUVBase
	{
	public:
		int Horizontal;
		int Vertical;
		ParticleSubUVMode Mode;
		float Cycles;

		ParticleModuleSubUV() :
			Horizontal(1),
			Vertical(1),
			Mode(E_PARTICLE_SUB_UV_MODE_LINEAR),
			Cycles(1.0f) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_SUBUV; }
		/*
		* Set random frame for particle if random and 0 if linear
		*/
		void Spawn(Particle& aParticle) override
		{
			if (Mode == E_PARTICLE_SUB_UV_MODE_LINEAR)
			{
				aParticle.frame = 0;
			} else
			{
				aParticle.frame = Math::TruncToInt(Random::range(0.0f, float(Horizontal * Vertical)));
			}
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override
		{
			if (Mode == E_PARTICLE_SUB_UV_MODE_LINEAR)
			{
				aParticle.frame = Math::TruncToInt(floor(Horizontal * Vertical * aParticle.percent * Cycles));
			}
		}

		~ParticleModuleSubUV() {}
	};

}









