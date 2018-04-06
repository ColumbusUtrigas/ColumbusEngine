#include <Audio/AudioSource.h>

namespace Columbus
{

	AudioSource::AudioSource() :
		SoundClip(nullptr),
		Gain(1.0f),
		Pitch(1.0f),
		MinDistance(0.01f),
		MaxDistance(1000.0f),
		Rolloff(1.0f),
		Looping(false)
	{
		SoundClip = new Sound();

		SetMinDistance(0.0f);
		SetMaxDistance(100.0f);
		SetRolloff(1.0f);
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

	void AudioSource::SetMinDistance(float InDistance)
	{
		this->MinDistance = InDistance;
	}

	void AudioSource::SetMaxDistance(float InDistance)
	{
		this->MaxDistance = InDistance;
	}

	void AudioSource::SetRolloff(float InRolloff)
	{
		this->Rolloff = InRolloff;
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

	float AudioSource::GetMinDistance() const
	{
		return MinDistance;
	}

	float AudioSource::GetMaxDistance() const
	{
		return MaxDistance;
	}

	float AudioSource::GetRolloff() const
	{
		return Rolloff;
	}

	bool AudioSource::GetLooping() const
	{
		return Looping;
	}

	AudioSource::~AudioSource() {}

}











