#include <Common/Sound/SoundUtil.h>
#include <System/Assert.h>
#include <cstring>

namespace Columbus
{

	void SoundUtil::Resample(Sound::Frame* src, Sound::Frame* dst, int from_samples, int from_freq, int to_freq)
	{
		if (from_freq == to_freq)
		{
			// so, there is nothing to do, just copy samples
			memcpy(dst, src, from_samples * sizeof(Sound::Frame));
			return;
		}

		if (from_freq * 2 == to_freq)
		{
			// 11025->22050 or 22050 to 44100
			for (int i = 0; i < from_samples; i++)
			{
				dst[i * 2 + 0] = src[i];
				dst[i * 2 + 1] = src[i];
			}

			return;
		}

		if (from_freq * 4 == to_freq)
		{
			// 11025->44100
			for (int i = 0; i < from_samples; i++)
			{
				dst[i * 4 + 0] = src[i];
				dst[i * 4 + 1] = src[i];
				dst[i * 4 + 2] = src[i];
				dst[i * 4 + 3] = src[i];
			}

			return;
		}

		COLUMBUS_ASSERT_MESSAGE(false, "Sound module: ResampleAudio(): invalid conversion");
	}

}


