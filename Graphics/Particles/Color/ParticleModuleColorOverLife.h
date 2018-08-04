#pragma once

#include <Graphics/Particles/Color/ParticleModuleColorBase.h>
#include <Math/InterpolationCurve.h>

namespace Columbus
{

	class ParticleModuleColorOverLife : public ParticleModuleColorBase
	{
	public:
		InterpolationCurve<Vector4> ColorCurve;
	public:
		ParticleModuleColorOverLife() {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Color; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.Color = ColorCurve.Interpolate(0.0f);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			OutParticle.Color = ColorCurve.Interpolate(OutParticle.percent);
		}

		~ParticleModuleColorOverLife() override {}
	};

}





