#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleSizeOverLife : public ParticleModule
	{
	public:
		InterpolationCurve<Vector3> SizeCurve;
	public:
		ParticleModuleSizeOverLife() {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Size; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.Size = SizeCurve.Interpolate(0.0f);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			OutParticle.Size = SizeCurve.Interpolate(OutParticle.percent);
		}

		~ParticleModuleSizeOverLife() override { }
	};

}




