#pragma once

#include <Audio/AudioMixer.h>

namespace Columbus
{

	class AudioPlayer
	{
	public:
		AudioPlayer(uint16 Channels, uint32 Frequency, AudioMixer* Mixer);
		~AudioPlayer();

		void Play();
		void Stop();
	};

}


