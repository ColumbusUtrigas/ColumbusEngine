#pragma once

#include <Audio/AudioPlayer.h>
#include <Audio/AudioMixer.h>

namespace Columbus
{

	/**
	* @brief Wrapper aroung AudioPlayer and AudioMixer.
	*/
	class AudioSystem
	{
	private:
		friend class Renderer;

		AudioPlayer* Player;
		AudioMixer Mixer;
	public:
		AudioSystem();

		void Clear();

		void AddSource(AudioSource* Source);
		void SetListener(AudioListener Listener);

		bool HasSource(AudioSource* Source);
		void SetSpeed(float Speed);
		float GetSpeed() const;

		void Play();
		void Stop();

		~AudioSystem();
	};

}


