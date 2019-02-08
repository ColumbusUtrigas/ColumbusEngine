#pragma once

#include <Graphics/Particles/ParticleModule.h>
#include <Common/Noise/OctaveNoise.h>

namespace Columbus
{

	class ParticleModuleNoise : public ParticleModule
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
			Amplitude(1.0f)
		{ }
		/*
		* For determening module type
		*/
		Type GetType() const override { return Type::Noise; }
		/*
		* Set parameter for particle
		*/
		void Spawn(Particle& OutParticle) override
		{
			if (Active)
			{
				Noise.Octaves = Octaves;
				Noise.Lacunarity = Lacunarity;
				Noise.Persistence = Persistence;
				Noise.BaseFrequency= Frequency;
				Noise.BaseAmplitude = Amplitude;
			}
		}
		/*
		* Update particle parameter
		*/
		void Update(Particle& OutParticle) override
		{
			if (Active)
			{
				VNoise.X = Noise.Noise(OutParticle.noise[0], OutParticle.noise[1], OutParticle.noise[2]);
				VNoise.Y = Noise.Noise(OutParticle.noise[3], OutParticle.noise[4], OutParticle.noise[5]);
				VNoise.Z = Noise.Noise(OutParticle.noise[6], OutParticle.noise[7], OutParticle.noise[8]);

				OutParticle.NoiseModifier = VNoise * Strength;
			}
		}

		~ParticleModuleNoise() override { }
	};

}







