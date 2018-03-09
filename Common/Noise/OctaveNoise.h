#pragma once

#include <Common/Noise/PerlinNoise.h>

namespace Columbus
{

	class OctaveNoise
	{
	private:
		PerlinNoise* mPerlinSource = nullptr;

		unsigned int mOctaves = 8;
		float mPersistence = 0.5;
		float mLacunarity = 2.0;
		float mBaseFrequency = 1.0;
		float mBaseAmplitude = 1.0;
	public:
		OctaveNoise() :
			mOctaves(8),
			mPersistence(0.5),
			mLacunarity(2.0),
			mBaseFrequency(1.0),
			mBaseAmplitude(1.0)
		{
			mPerlinSource = new PerlinNoise();
		}

		float noise(float sample_x, float sample_y, float sample_z)
		{
			float sum = 0;
			float freq = mBaseFrequency;
			float amp = mBaseAmplitude;

			size_t i;

			for (i = 0; i < mOctaves; i++)
			{
				sum += mPerlinSource->noise(sample_x * freq, sample_y * freq, sample_z * freq) * amp;

				freq *= mLacunarity;
				amp *= mPersistence;
			}

			return sum;
		}

		void setOctaves(unsigned int o)
		{
			mOctaves = o;
		}

		void setPersistence(float p)
		{
			mPersistence = p;
		}

		void setLacunarity(float l)
		{
			mLacunarity = l;
		}
		
		void setFrequency(float f)
		{
			mBaseFrequency = f;
		}

		void setAmplitude(float a)
		{
			mBaseAmplitude = a;
		}

		~OctaveNoise()
		{
			delete mPerlinSource;
		}
	};

}




