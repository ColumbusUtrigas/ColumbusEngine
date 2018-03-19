#pragma once

#include <Graphics/Particles/Noise/ParticleModuleNoiseBase.h>
#include <Common/Noise/OctaveNoise.h>

namespace Columbus
{

	class ParticleModuleNoise : public ParticleModuleNoiseBase
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
		void Spawn(Particle& aParticle) override
		{
			if (Active)
			{
				Noise.setOctaves(Octaves);
				Noise.setLacunarity(Lacunarity);
				Noise.setPersistence(Persistence);
				Noise.setFrequency(Frequency);
				Noise.setAmplitude(Amplitude);
			}
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& aParticle) override
		{
			if (Active)
			{
				VNoise.x = Noise.noise(aParticle.noise[0], aParticle.noise[1], aParticle.noise[2]);
				VNoise.y = Noise.noise(aParticle.noise[3], aParticle.noise[4], aParticle.noise[5]);
				VNoise.z = Noise.noise(aParticle.noise[6], aParticle.noise[7], aParticle.noise[8]);

				aParticle.NoiseModifier = VNoise * Strength;
			}
		}

		~ParticleModuleNoise() override {}
	};

}







