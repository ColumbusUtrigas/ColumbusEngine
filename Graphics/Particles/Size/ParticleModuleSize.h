#pragma once

#include <Graphics/Particles/Size/ParticleModuleSizeBase.h>

namespace Columbus
{

	class ParticleModuleSize : public ParticleModuleSizeBase
	{
	public:
		Vector3 Min;
		Vector3 Max;
	public:
		ParticleModuleSize() :
			Min(Vector3(1, 1, 1)),
			Max(Vector3(1, 1, 1))
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
			aParticle.size = Vector3::Random(Min, Max);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override { }

		~ParticleModuleSize() override { }
	};

}






