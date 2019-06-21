#pragma once

#include <Graphics/Particles/ParticleContainer.h>
#include <Common/Noise/OctaveNoise.h>

namespace Columbus
{

	class ParticleModuleNoise
	{
	private:
		OctaveNoise Noise;
		Vector3 VNoise;
	public:
		float Strength;
		int Octaves;
		float Lacunarity;
		float Persistence;
		float Frequency;
		float Amplitude;
	public:
		ParticleModuleNoise() :
			Strength(1.0f),
			Octaves(1),
			Lacunarity(2.0f),
			Persistence(0.5f),
			Frequency(1.0f),
			Amplitude(1.0f) {}

		~ParticleModuleNoise() { }
	};

}


