#pragma once

#include <Graphics/Particles/ParticleModule.h>

namespace Columbus
{

	class ParticleModuleSubUV : public ParticleModule
	{
	public:
		/*
		* If nedded to create particle spritesheet animation or random texture from sheet
		*/
		enum class SubUVMode
		{
			Linear,
			Random
		};
	public:
		int Horizontal;
		int Vertical;
		SubUVMode Mode;
		float Cycles;
	public:
		ParticleModuleSubUV() :
			Horizontal(1),
			Vertical(1),
			Mode(SubUVMode::Linear),
			Cycles(1.0f) {}
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::SubUV; }
		/*
		* Set random frame for particle if random and 0 if linear
		*/
		void Spawn(Particle& OutParticle) override
		{
			if (Mode == SubUVMode::Linear)
			{
				OutParticle.frame = 0;
			}
			else
			{
				OutParticle.frame = Math::TruncToInt(Random::Range(0.0f, float(Horizontal * Vertical)));
			}
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			if (Mode == SubUVMode::Linear)
			{
				OutParticle.frame = Math::TruncToInt(floor(Horizontal * Vertical * OutParticle.percent * Cycles));
			}
		}

		~ParticleModuleSubUV() {}
	};

}









