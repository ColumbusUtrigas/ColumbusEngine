#include <Audio/AudioSource.h>

namespace Columbus
{

	AudioSource::AudioSource() :
		SoundClip(nullptr),
		Gain(1.0f),
		Pitch(1.0f),
		Looping(false)
	{
		SoundClip = new Sound();
	}

	void AudioSource::Play() {}
	void AudioSource::Pause() {}
	void AudioSource::Stop() {}
	void AudioSource::Rewind() {}

	void AudioSource::SetSound(Sound* InSound)
	{
		this->SoundClip = InSound;
	}

	void AudioSource::SetPosition(Vector3 InPosition)
	{
		this->Position = InPosition;
	}

	void AudioSource::SetVelocity(Vector3 InVelocity)
	{
		this->Velocity = InVelocity;
	}

	void AudioSource::SetDirection(Vector3 InDirection)
	{
		this->Direction = InDirection;
	}

	void AudioSource::SetGain(float InGain)
	{
		this->Gain = InGain;
	}

	void AudioSource::SetPitch(float InPitch)
	{
		this->Pitch = InPitch;
	}

	void AudioSource::SetLooping(bool InLooping)
	{
		this->Looping = InLooping;
	}

	Sound* AudioSource::GetSound() const
	{
		return SoundClip;
	}

	Vector3 AudioSource::GetPosition() const
	{
		return Position;
	}

	Vector3 AudioSource::GetVelocity() const
	{
		return Velocity;
	}

	Vector3 AudioSource::GetDirection() const
	{
		return Direction;
	}

	float AudioSource::GetGain() const
	{
		return Gain;
	}

	float AudioSource::GetPitch() const
	{
		return Pitch;
	}

	bool AudioSource::GetLooping() const
	{
		return Looping;
	}

	AudioSource::~AudioSource() {}

}











