#pragma once

#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>

namespace Columbus
{

	class ParticleModuleLocationCircle : public ParticleModuleLocationBase
	{
	public:
		float Radius;
		bool EmitFromShell;
	public:
		ParticleModuleLocationCircle() :
			Radius(1.0f),
			EmitFromShell(false)
		{ }
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Location; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			float ang = Random::Range(0.0f, 6.283185306f);
			float rad = Random::Range(0.0f, Radius);

			if (EmitFromShell) rad = Radius;

			OutParticle.startPos.X = rad * Math::Cos(ang);
			OutParticle.startPos.Y = 0.0;
			OutParticle.startPos.Z = rad * Math::Sin(ang);
		}

		~ParticleModuleLocationCircle() override { }
	};

}





