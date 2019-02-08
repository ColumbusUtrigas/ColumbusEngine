#pragma once

#include <Audio/AudioMixer.h>

namespace Columbus
{

	class AudioPlayer
	{
	public:
		AudioPlayer(uint16 Channels, uint32 Frequency, AudioMixer* Mixer);

		void Play();
		void Stop();
	};

}


