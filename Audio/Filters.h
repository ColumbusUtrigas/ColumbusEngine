#pragma once

#include "Common/Sound/Sound.h"

namespace Columbus
{

	struct AudioFilterLowPass
	{
		float Buffer[2][4]{ 0 };
		float Frequency = 1.0f;

		// T: int16 or int32
		template <typename T>
		inline void Process(T& x, float* out)
		{
			out[0] += Frequency * (float(x) - out[0]);
			out[1] += Frequency * (out[0] - out[1]);
			out[2] += Frequency * (out[1] - out[2]);
			out[3] += Frequency * (out[2] - out[3]);
			x = T(out[3]);
		}

		void Process(Sound::FrameHight* frames, int count)
		{
			for (int i = 0; i < count; i++)
			{
				Process(frames[i].L, Buffer[0]);
				Process(frames[i].R, Buffer[1]);
			}
		}

		void Process(Sound::Frame* frames, int count)
		{
			for (int i = 0; i < count; i++)
			{
				Process(frames[i].L, Buffer[0]);
				Process(frames[i].R, Buffer[1]);
			}
		}
	};

}
