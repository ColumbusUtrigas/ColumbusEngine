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
		Type GetType() const override { return Type::Location; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			float rad = Random::Range(0.0f, Radius);
			float phi = Random::Range(0.0f, 6.283185306f);
			float tht = Random::Range(0.0f, 3.141592653f);

			if (EmitFromShell) rad = Radius;

			OutParticle.startPos.X = rad * Math::Cos(phi) * Math::Sin(tht);
			OutParticle.startPos.Y = rad * Math::Sin(phi) * Math::Sin(tht);
			OutParticle.startPos.Z = rad * Math::Cos(tht);
		}

		~ParticleModuleLocationSphere() override {}
	};

}





