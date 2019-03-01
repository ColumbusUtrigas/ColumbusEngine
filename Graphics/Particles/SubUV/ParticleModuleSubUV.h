#pragma once

#include <Graphics/Particles/ParticleModule.h>
#include <Graphics/Particles/ParticleContainer.h>

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
		float Cycles;

		SubUVMode Mode;
	public:
		ParticleModuleSubUV() : Horizontal(1), Vertical(1), Mode(SubUVMode::Linear), Cycles(1.0f) {}
		
		Type GetType() const override { return Type::SubUV; }
		
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
		
		void Update(Particle& OutParticle) override
		{
			if (Mode == SubUVMode::Linear)
			{
				OutParticle.frame = Math::TruncToInt(floor(Horizontal * Vertical * OutParticle.percent * Cycles));
			}
		}

		void Spawn(ParticleContainer& Container, size_t Spawn) const;
		void Update(ParticleContainer& Container) const;

		~ParticleModuleSubUV() {}
	};

}


