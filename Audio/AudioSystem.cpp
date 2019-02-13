#include <Audio/AudioSystem.h>

namespace Columbus
{

	AudioSystem::AudioSystem()
	{
		Player = new AudioPlayer(2, 44100, &Mixer);
	}

	void AudioSystem::AddSource(AudioSource* Source)
	{
		Mixer.AddSource(Source);
	}

	void AudioSystem::SetListener(AudioListener Listener)
	{
		Mixer.SetListener(Listener);
	}

	bool AudioSystem::HasSource(AudioSource* Source)
	{
		return Mixer.HasSource(Source);
	}

	void AudioSystem::SetSpeed(float Speed)
	{
		Mixer.SetSpeed(Speed);
	}

	float AudioSystem::GetSpeed() const
	{
		return Mixer.GetSpeed();
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
	}

}


