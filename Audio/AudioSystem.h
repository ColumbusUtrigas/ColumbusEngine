#pragma once

#include <Audio/AudioPlayer.h>
#include <Audio/AudioMixer.h>
#include <memory>

namespace Columbus
{

	/**
	* @brief Wrapper aroung AudioPlayer and AudioMixer.
	*/
	class AudioSystem
	{
	private:
		friend class Renderer;

		std::unique_ptr<AudioPlayer> Player;
		AudioMixer Mixer;
	public:
		AudioSystem();

		void Clear();

		void AddSource(std::shared_ptr<AudioSource> Source);
		void SetListener(AudioListener Listener);

		void SetSpeed(float Speed);
		float GetSpeed() const;

		void Play();
		void Stop();

		~AudioSystem();
	};

}
