#include <Audio/AudioSystem.h>

namespace Columbus
{

	AudioSystem::AudioSystem()
	{
		Player = std::make_unique<AudioPlayer>(2, 44100, &MasterMixer);
	}

	void AudioSystem::Clear()
	{
		MasterMixer.Clear();
	}

	void AudioSystem::Play()
	{
		Player->Play();
	}

	void AudioSystem::Stop()
	{
		Player->Stop();
	}

	AudioSystem::~AudioSystem()
	{
	}

}
