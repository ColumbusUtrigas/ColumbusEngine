#pragma once

#include <Common/Sound/Sound.h>

namespace Columbus
{

	struct SoundUtil
	{
		static void Resample(Sound::Frame* src, Sound::Frame* dst, int from_samples, int from_freq, int to_freq);
	};

}


