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

		void AddSource(AudioSource* Source);
		void SetListener(AudioListener Listener);

		void Play();
		void Stop();

		~AudioSystem();
	};

}














