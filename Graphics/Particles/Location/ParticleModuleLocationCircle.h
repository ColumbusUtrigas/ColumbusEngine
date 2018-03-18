#pragma once

#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>

namespace Columbus
{

	class ParticleModuleLocationCircle : public ParticleModuleLocationBase
	{
	public:
		float Radius;
		bool EmitFromShell;

		ParticleModuleLocationCircle() :
			Radius(1.0f),
			EmitFromShell(false) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_LOCATION; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			float ang = Random::range(0.0, 6.283185306);
			float rad = Random::range(0.0, Radius);

			if (EmitFromShell) rad = Radius;

			aParticle.startPos.x = rad * cos(ang);
			aParticle.startPos.y = 0.0;
			aParticle.startPos.z = rad * sin(ang);
		}

		~ParticleModuleLocationCircle() override {}
	};

}





