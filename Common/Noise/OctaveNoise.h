#pragma once

#include <Common/Noise/PerlinNoise.h>
#include <Core/Types.h>

namespace Columbus
{

	class OctaveNoise
	{
	private:
		PerlinNoise Perlin;
	public:
		uint32 Octaves = 8;
		float Persistence = 0.5;
		float Lacunarity = 2.0;
		float BaseFrequency = 1.0;
		float BaseAmplitude = 1.0;
	public:
		OctaveNoise() {}

		float Noise(float X, float Y, float Z)
		{
			float Sum = 0;
			float Frequency = BaseFrequency;
			float Amplitude = BaseAmplitude;

			for (uint32 i = 0; i < Octaves; i++)
			{
				Sum += Perlin.Noise(X * Frequency, Y * Frequency, Z * Frequency) * Amplitude;

				Frequency *= Lacunarity;
				Amplitude *= Persistence;
			}

			return Sum;
		}
	};

}


