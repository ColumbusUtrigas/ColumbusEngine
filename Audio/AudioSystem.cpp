#include <Audio/AudioSystem.h>

namespace Columbus
{

	AudioSystem::AudioSystem()
	{
		Player = new AudioPlayer(2, 44100, &Mixer);
	}

	void AudioSystem::Clear()
	{
		Mixer.Clear();
	}

	void AudioSystem::AddSource(std::shared_ptr<AudioSource> Source)
	{
		Mixer.AddSource(Source);
	}

	void AudioSystem::SetListener(AudioListener Listener)
	{
		Mixer.SetListener(Listener);
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


