#pragma once

#include <Graphics/Particles/Size/ParticleModuleSizeBase.h>

namespace Columbus
{

	class ParticleModuleSizeOverLife : public ParticleModuleSizeBase
	{
	public:
		Vector3 MinStart;
		Vector3 MaxStart;
		Vector3 MinFinal;
		Vector3 MaxFinal;
	public:
		ParticleModuleSizeOverLife() : 
			MinStart(1, 1, 1),
			MaxStart(1, 1, 1),
			MinFinal(1, 1, 1),
			MaxFinal(1, 1, 1)
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
			OutParticle.startSize = Vector3::Random(MinStart, MaxStart);
			OutParticle.finalSize = Vector3::Random(MinFinal, MaxFinal);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			OutParticle.size = OutParticle.startSize * (1 - OutParticle.percent) + OutParticle.finalSize * OutParticle.percent;
		}

		~ParticleModuleSizeOverLife() override { }
	};

}




