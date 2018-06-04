#include <Audio/AudioSystem.h>

namespace Columbus
{

	AudioSystem::AudioSystem()
	{
		Mixer = new AudioMixer();
		Player = new AudioPlayer(2, 44100, Mixer);
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
		delete Player;
		delete Mixer;
	}

}










