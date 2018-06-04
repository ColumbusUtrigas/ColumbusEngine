#pragma once

#include <Audio/AudioPlayer.h>
#include <Audio/AudioMixer.h>

namespace Columbus
{

	class AudioSystem
	{
	private:
		AudioPlayer* Player;
		AudioMixer* Mixer;
	public:
		AudioSystem();

		void Play();

		void Stop();

		~AudioSystem();
	};

}














