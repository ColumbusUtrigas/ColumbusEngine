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
		Type GetType() const override { return Type::Size; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.Size = Vector3::Random(Min, Max);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override { }

		~ParticleModuleSize() override { }
	};

}






