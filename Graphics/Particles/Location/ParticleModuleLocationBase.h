#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleLocationBase : public ParticleModule
	{
	public:
		ParticleModuleLocationBase() {}
		/*
		* Set parameter for particle
		* Set start position of the particle
		* In child classes this method overrides and sets start position in some shape
		*/
		virtual void Spawn(Particle& aParticle) override
		{
			aParticle.startPos = Vector3(0, 0, 0);
		}

		virtual ~ParticleModuleLocationBase() override {}
	};

}






