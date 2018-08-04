#pragma once

#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>

namespace Columbus
{

	class ParticleModuleLocationBox : public ParticleModuleLocationBase
	{
	public:
		Vector3 Size;
		bool EmitFromShell;
	public:
		ParticleModuleLocationBox() :
			Size(0, 0, 0),
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
			if (EmitFromShell)
			{
				switch (rand() % 3)
				{
				case 0:
					OutParticle.startPos.X = Size.X * 0.5f * ((rand() % 2) ? -1 : 1);
					OutParticle.startPos.Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
					OutParticle.startPos.Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 1:
					OutParticle.startPos.X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
					OutParticle.startPos.Y = Size.Y * 0.5f * ((rand() % 2) ? -1 : 1);
					OutParticle.startPos.Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 2:
					OutParticle.startPos.X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
					OutParticle.startPos.Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
					OutParticle.startPos.Z = Size.Z * 0.5f * ((rand() % 2) ? -1 : 1);
					break;
				}
			}
			else
			{
				OutParticle.startPos.X = Random::Range(-Size.X * 0.5f, Size.X * 0.5f);
				OutParticle.startPos.Y = Random::Range(-Size.Y * 0.5f, Size.Y * 0.5f);
				OutParticle.startPos.Z = Random::Range(-Size.Z * 0.5f, Size.Z * 0.5f);
			}
		}

		~ParticleModuleLocationBox() override { }
	};

}






