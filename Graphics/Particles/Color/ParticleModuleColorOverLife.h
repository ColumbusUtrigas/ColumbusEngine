#pragma once

#include <Graphics/Particles/Color/ParticleModuleColorBase.h>
#include <Math/InterpolationCurve.h>

namespace Columbus
{

	class ParticleModuleColorOverLife : public ParticleModuleColorBase
	{
	public:
		InterpolationCurve<Vector4> ColorCurve;
		Vector4 MinStart;
		Vector4 MaxStart;
		Vector4 MinFinal;
		Vector4 MaxFinal;

		ParticleModuleColorOverLife() :
			MinStart(Vector4(1, 1, 1, 1)),
			MaxStart(Vector4(1, 1, 1, 1)),
			MinFinal(Vector4(1, 1, 1, 1)),
			MaxFinal(Vector4(1, 1, 1, 1))
		{}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Color; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			OutParticle.startColor = Vector4::Random(MinStart, MaxStart);
			OutParticle.finalColor = Vector4::Random(MinFinal, MaxFinal);
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			OutParticle.color = ColorCurve.Interpolate(OutParticle.percent);
		}

		~ParticleModuleColorOverLife() override {}
	};

}





