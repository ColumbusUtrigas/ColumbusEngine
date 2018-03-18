#pragma once

#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>

namespace Columbus
{

	class ParticleModuleLocationSphere : public ParticleModuleLocationBase
	{
	public:
		float Radius;
		bool EmitFromShell;

		ParticleModuleLocationSphere() :
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
			float rad = Random::range(0.0, Radius);
			float phi = Random::range(0.0, 6.283185306);
			float tht = Random::range(0.0, 3.141592653);

			if (EmitFromShell) rad = Radius;

			aParticle.startPos.x = rad * cos(phi) * sin(tht);
			aParticle.startPos.y = rad * sin(phi) * sin(tht);
			aParticle.startPos.z = rad * cos(tht);
		}

		~ParticleModuleLocationSphere() override {}
	};

}





