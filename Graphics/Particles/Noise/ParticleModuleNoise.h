#pragma once

#include <Graphics/Particles/Noise/ParticleModuleNoiseBase.h>

namespace Columbus
{

	class ParticleModuleNoise : public ParticleModuleNoiseBase
	{
	public:
		float Strength;
		int Octaves;
		float Lacunarity;
		float Persistence;
		float Frequency;
		float Amplitude;

		ParticleModuleNoise() :
			Strength(1.0f),
			Octaves(1),
			Lacunarity(2.0f),
			Persistence(0.5f),
			Frequency(1.0f),
			Amplitude(1.0f) {}
		/*
		* For determening module type
		*/
		ParticleModuleType GetType() const override { return E_PARTICLE_MODULE_NOISE; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& aParticle) override {}

		~ParticleModuleNoise() override {}
	};

}







