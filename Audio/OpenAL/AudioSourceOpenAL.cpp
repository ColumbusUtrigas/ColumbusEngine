#include <Audio/OpenAL/AudioSourceOpenAL.h>
#include <AL/al.h>

namespace Columbus
{

	AudioSourceOpenAL::AudioSourceOpenAL()
	{
		SoundClip = new Sound();

		alGenBuffers(1, &OpenALBuffer);
		alGenSources(1, &OpenALSource);
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

		if (SoundClip != nullptr)
		{
			alDeleteBuffers(1, &OpenALBuffer);
			alGenBuffers(1, &OpenALBuffer);

			if (SoundClip->GetBuffer() != nullptr && SoundClip->GetBufferSize() != 0 && SoundClip->GetFrequency() != 0 && SoundClip->GetChannelsCount() != 0)
			{
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
			}
		}
	}

	void AudioSourceOpenAL::SetPosition(Vector3 InPosition)
	{
		this->Position = InPosition;
		alSource3f(OpenALSource, AL_POSITION, Position.x, Position.y, Position.z);
	}

	void AudioSourceOpenAL::SetVelocity(Vector3 InVelocity)
	{
		this->Velocity = InVelocity;
		alSource3f(OpenALSource, AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
	}

	void AudioSourceOpenAL::SetDirection(Vector3 InDirection)
	{
		this->Direction = InDirection;
		alSource3f(OpenALSource, AL_DIRECTION, Direction.x, Direction.y, Direction.z);
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

	void AudioSourceOpenAL::SetLooping(bool InLooping)
	{
		this->Looping = InLooping;
		alSourcei(OpenALSource, AL_LOOPING, Looping);
	}

	AudioSourceOpenAL::~AudioSourceOpenAL()
	{
		alDeleteBuffers(1, &OpenALBuffer);
		alDeleteSources(1, &OpenALSource);
	}	

}














