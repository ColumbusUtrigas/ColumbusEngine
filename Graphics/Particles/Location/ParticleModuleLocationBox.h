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
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_LOCATION; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override
		{
			if (EmitFromShell)
			{
				switch (rand() % 3)
				{
				case 0:
					aParticle.startPos.X = Size.X * 0.5f * ((rand() % 2) ? -1 : 1);
					aParticle.startPos.Y = Random::range(-Size.Y * 0.5f, Size.Y * 0.5f);
					aParticle.startPos.Z = Random::range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 1:
					aParticle.startPos.X = Random::range(-Size.X * 0.5f, Size.X * 0.5f);
					aParticle.startPos.Y = Size.Y * 0.5f * ((rand() % 2) ? -1 : 1);
					aParticle.startPos.Z = Random::range(-Size.Z * 0.5f, Size.Z * 0.5f);
					break;
				case 2:
					aParticle.startPos.X = Random::range(-Size.X * 0.5f, Size.X * 0.5f);
					aParticle.startPos.Y = Random::range(-Size.Y * 0.5f, Size.Y * 0.5f);
					aParticle.startPos.Z = Size.Z * 0.5f * ((rand() % 2) ? -1 : 1);
					break;
				}
			}
			else
			{
				aParticle.startPos.X = Random::range(-Size.X * 0.5f, Size.X * 0.5f);
				aParticle.startPos.Y = Random::range(-Size.Y * 0.5f, Size.Y * 0.5f);
				aParticle.startPos.Z = Random::range(-Size.Z * 0.5f, Size.Z * 0.5f);
			}
		}

		~ParticleModuleLocationBox() override { }
	};

}






