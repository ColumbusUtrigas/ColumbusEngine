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

	public:
		AudioMixer MasterMixer;
	public:
		AudioSystem();

		void Clear();

		void Play();
		void Stop();

		~AudioSystem();
	};

}
