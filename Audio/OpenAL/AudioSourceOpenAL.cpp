#include <Audio/OpenAL/AudioSourceOpenAL.h>
#include <al.h>

namespace Columbus
{

	AudioSourceOpenAL::AudioSourceOpenAL()
	{
		SoundClip = new Sound();
	}

	void AudioSourceOpenAL::Play()
	{
		alSourcePlay(OpenALSource);
	}

	void AudioSourceOpenAL::Pause()
	{
		alSourcePause(OpenALSource);
	}

	void AudioSourceOpenAL::Stop()
	{
		alSourceStop(OpenALSource);
	}

	void AudioSourceOpenAL::Rewind()
	{
		alSourceRewind(OpenALSource);
	}

	void AudioSourceOpenAL::SetSound(Sound* InSound)
	{
		this->SoundClip = InSound;

		alSourcei(OpenALSource, AL_BUFFER, 0);
		alDeleteSources(1, &OpenALSource);
		alDeleteBuffers(1, &OpenALBuffer);

		if (SoundClip != nullptr)
		{
			if (SoundClip->GetBuffer() != nullptr && SoundClip->GetBufferSize() != 0 && SoundClip->GetFrequency() != 0 && SoundClip->GetChannelsCount() != 0)
			{
				alGenSources(1, &OpenALSource);
				alGenBuffers(1, &OpenALBuffer);

				switch (SoundClip->GetChannelsCount())
				{
				case 1:
					alBufferData(OpenALBuffer, AL_FORMAT_MONO16, SoundClip->GetBuffer(), SoundClip->GetBufferSize(), SoundClip->GetFrequency());
					break;
				case 2:
					alBufferData(OpenALBuffer, AL_FORMAT_STEREO16, SoundClip->GetBuffer(), SoundClip->GetBufferSize(), SoundClip->GetFrequency());
					break;
				}

				alSourcei(OpenALSource, AL_BUFFER, OpenALBuffer);

				alSource3f(OpenALSource, AL_POSITION, Position.X, Position.Y, Position.Z);
				alSource3f(OpenALSource, AL_VELOCITY, Velocity.X, Velocity.Y, Velocity.Z);
				alSource3f(OpenALSource, AL_DIRECTION, Direction.X, Direction.Y, Direction.Z);
				alSourcef(OpenALSource, AL_GAIN, Gain);
				alSourcef(OpenALSource, AL_PITCH, Pitch);
				alSourcef(OpenALSource, AL_REFERENCE_DISTANCE, MinDistance);
				alSourcef(OpenALSource, AL_MAX_DISTANCE, MaxDistance);
				alSourcef(OpenALSource, AL_ROLLOFF_FACTOR, Rolloff);
				alSourcei(OpenALSource, AL_LOOPING, Looping);
			}
		}
	}

	void AudioSourceOpenAL::SetPosition(Vector3 InPosition)
	{
		this->Position = InPosition;
		alSource3f(OpenALSource, AL_POSITION, Position.X, Position.Y, Position.Z);
	}

	void AudioSourceOpenAL::SetVelocity(Vector3 InVelocity)
	{
		this->Velocity = InVelocity;
		alSource3f(OpenALSource, AL_VELOCITY, Velocity.X, Velocity.Y, Velocity.Z);
	}

	void AudioSourceOpenAL::SetDirection(Vector3 InDirection)
	{
		this->Direction = InDirection;
		alSource3f(OpenALSource, AL_DIRECTION, Direction.X, Direction.Y, Direction.Z);
	}

	void AudioSourceOpenAL::SetGain(float InGain)
	{
		this->Gain = InGain;
		alSourcef(OpenALSource, AL_GAIN, Gain);
	}

	void AudioSourceOpenAL::SetPitch(float InPitch)
	{
		this->Pitch = InPitch;
		alSourcef(OpenALSource, AL_PITCH, Pitch);
	}

	void AudioSourceOpenAL::SetMinDistance(float InDistance)
	{
		this->MinDistance = InDistance;
		alSourcef(OpenALSource, AL_REFERENCE_DISTANCE, MinDistance);
	}

	void AudioSourceOpenAL::SetMaxDistance(float InDistance)
	{
		this->MaxDistance = InDistance;
		alSourcef(OpenALSource, AL_MAX_DISTANCE, MaxDistance);
	}

	void AudioSourceOpenAL::SetRolloff(float InRolloff)
	{
		this->Rolloff = InRolloff;
		alSourcef(OpenALSource, AL_ROLLOFF_FACTOR, Rolloff);
	}

	void AudioSourceOpenAL::SetLooping(bool InLooping)
	{
		this->Looping = InLooping;
		alSourcei(OpenALSource, AL_LOOPING, Looping);
	}

	AudioSourceOpenAL::~AudioSourceOpenAL()
	{
		alSourcei(OpenALSource, AL_BUFFER, 0);
		alDeleteSources(1, &OpenALSource);
		alDeleteBuffers(1, &OpenALBuffer);
	}	

}














