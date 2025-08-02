#pragma once

#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	/*
	* If needed to create particle spritesheet animation or random texture from sheet
	*/
	enum class EParticleSubUVMode
	{
		Linear,
		Random
	};

	class ParticleModuleSubUV
	{
	public:
		int Horizontal;
		int Vertical;
		float Cycles;

		EParticleSubUVMode Mode;
	public:
		ParticleModuleSubUV() : Horizontal(1), Vertical(1), Cycles(1.0f), Mode(EParticleSubUVMode::Linear) {}

		void Spawn(ParticleContainer& Container, size_t Spawn) const;
		void Update(ParticleContainer& Container) const;

		~ParticleModuleSubUV() {}
	};

}


