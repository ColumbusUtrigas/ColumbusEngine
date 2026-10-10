#include <Audio/AudioSystem.h>

namespace Columbus
{

	AudioSystem::AudioSystem(bool EnablePlayback)
	{
		if (EnablePlayback) Player = std::make_unique<AudioPlayer>(2, 44100, &MasterMixer);
	}

	void AudioSystem::Clear()
	{
		MasterMixer.Clear();
	}

	void AudioSystem::Play()
	{
		if (Player) Player->Play();
	}

	void AudioSystem::Stop()
	{
		if (Player) Player->Stop();
	}

	AudioSystem::~AudioSystem()
	{
		if (Player)
		{
			Player->Stop();
			Player.reset();
		}
	}

}
