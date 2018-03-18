#pragma once

#include <Graphics/Particles/Location/ParticleModuleLocationBase.h>

namespace Columbus
{

	class ParticleModuleLocationBox : public ParticleModuleLocationBase
	{
	public:
		Vector3 Size;
		bool EmitFromShell;

		ParticleModuleLocationBox() :
			Size(Vector3(0, 0, 0)),
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
			if (EmitFromShell)
			{
				switch (rand() % 3)
				{
				case 0:
					aParticle.startPos.x = Size.x / 2 * ((rand() % 2) ? -1 : 1);
					aParticle.startPos.y = Random::range(-Size.y / 2, Size.y / 2);
					aParticle.startPos.z = Random::range(-Size.z / 2, Size.z / 2);
					break;
				case 1:
					aParticle.startPos.x = Random::range(-Size.x / 2, Size.x / 2);
					aParticle.startPos.y = Size.y / 2 * ((rand() % 2) ? -1 : 1);
					aParticle.startPos.z = Random::range(-Size.z / 2, Size.z / 2);
					break;
				case 2:
					aParticle.startPos.x = Random::range(-Size.x / 2, Size.x / 2);
					aParticle.startPos.y = Random::range(-Size.y / 2, Size.y / 2);
					aParticle.startPos.z = Size.z / 2 * ((rand() % 2) ? -1 : 1);
					break;
				}
			}
			else
			{
				aParticle.startPos.x = Random::range(-Size.x / 2, Size.x / 2);
				aParticle.startPos.y = Random::range(-Size.y / 2, Size.y / 2);
				aParticle.startPos.z = Random::range(-Size.z / 2, Size.z / 2);
			}
		}

		~ParticleModuleLocationBox() override {}
	};

}






