#pragma once

#include <Common/Noise/PerlinNoise.h>

namespace Columbus
{

	class C_OctaveNoise
	{
	private:
		C_PerlinNoise* m_perlinSource = nullptr;

		unsigned int m_octaves = 8;
		float m_persistence = 0.5;
		float m_lacunarity = 2.0;
		float m_baseFrequency = 1.0;
		float m_baseAmplitude = 1.0;
	public:
		C_OctaveNoise() :
			m_octaves(1),
			m_lacunarity(2.0),
			m_persistence(0.5),
			m_baseFrequency(0.1),
			m_baseAmplitude(1.0)
		{
			m_perlinSource = new C_PerlinNoise();
		}

		float noise(float sample_x, float sample_y, float sample_z)
		{
			float sum = 0;
			float freq = m_baseFrequency;
			float amp = m_baseAmplitude;

			size_t i;

			for (i = 0; i < m_octaves; i++)
			{
				sum += m_perlinSource->noise(sample_x * freq, sample_y * freq, sample_z * freq) * amp;

				freq *= m_lacunarity;
				amp *= m_persistence;
			}

			return sum;
		}

		void setOctaves(unsigned int o)
		{
			m_octaves = o;
		}

		void setPersistence(float p)
		{
			m_persistence = p;
		}

		void setLacunarity(float l)
		{
			m_lacunarity = l;
		}
		
		void setFrequency(float f)
		{
			m_baseFrequency = f;
		}

		void setAmplitude(float a)
		{
			m_baseAmplitude = a;
		}

		~C_OctaveNoise()
		{
			delete m_perlinSource;
		}
};

}




