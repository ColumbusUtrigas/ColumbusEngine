#pragma once

#include <Graphics/Particles/ParticleContainer.h>

namespace Columbus
{

	class ParticleModuleSubUV
	{
	public:
		/*
		* If needed to create particle spritesheet animation or random texture from sheet
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

		void Spawn(ParticleContainer& Container, size_t Spawn) const;
		void Update(ParticleContainer& Container) const;

		~ParticleModuleSubUV() {}
	};

}


