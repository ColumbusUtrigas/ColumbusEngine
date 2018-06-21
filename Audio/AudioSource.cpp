#include <Audio/AudioSource.h>
#include <Core/Memory.h>

namespace Columbus
{

	AudioSource::AudioSource() :
		SoundClip(nullptr),
		Gain(1.0f),
		Pitch(1.0f),
		MinDistance(Math::Sqrt(8)),
		MaxDistance(100.0f),
		Rolloff(1.0f),
		Playing(false),
		Looping(false),
		Offset(0),
		Played(0.0),
		SoundMode(AudioSource::Mode::Sound3D)
	{
		SoundClip = new Sound();
	}

	void AudioSource::Play() { Playing = true; }
	void AudioSource::Pause() { Playing = false; }
	void AudioSource::Stop() { Playing = false; Offset = 0; }
	void AudioSource::Rewind() { Offset = 0; }

	void AudioSource::SetPlayedTime(double Time)
	{
		if (SoundClip != nullptr)
		{
			Played = Time;
			Offset = Time * SoundClip->GetFrequency();
			SoundClip->Seek(Offset);
		}
	}

	double AudioSource::GetPlayedTime() const
	{
		return Played;
	}

	void AudioSource::SetMode(AudioSource::Mode InMode) { SoundMode = InMode; }
	AudioSource::Mode AudioSource::GetMode() const { return SoundMode; }

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

	bool AudioSource::IsPlaying() const
	{
		return Playing;
	}

	bool AudioSource::IsLooping() const
	{
		return Looping;
	}

	void AudioSource::PrepareBuffer(Sound::Frame* Frames, uint32 Count)
	{
		if (Frames != nullptr && Count != 0)
		{
			if (Playing)
			{
				if (SoundClip != nullptr)
				{
					if (SoundClip->IsStreaming())
					{
						if (SoundClip->GetFrequency() != 0 &&
						    SoundClip->GetChannelsCount() != 0)
						{
							uint32 Decoded = SoundClip->Decode(Frames, Count, Offset);

							Offset += Count * SoundClip->GetChannelsCount();
							Played += 1.0 / SoundClip->GetFrequency() * Count;

							if (Decoded < Count)
							{
								SoundClip->Seek(0);
								Offset = 0;
								Played = 0.0;
							}
						}
					}
					else
					{
						if (SoundClip->GetBufferSize() != 0 &&
							SoundClip->GetFrequency() != 0 &&
							SoundClip->GetChannelsCount() != 0 &&
							SoundClip->GetBuffer() != nullptr)
						{
							uint32 FramesCount = 0;

							if (Offset >= SoundClip->GetBufferSize() / sizeof(int16) - Count * sizeof(Sound::Frame))
							{
								uint32 BufferSize = Offset - (SoundClip->GetBufferSize() / sizeof(int16) - Count * sizeof(Sound::Frame));
								FramesCount = BufferSize / sizeof(Sound::Frame);
							}
							else
							{
								FramesCount = Count;
							}

							const double TimeStep = 1.0 / SoundClip->GetFrequency();

							for (uint32 i = 0; i < FramesCount; i++)
							{
								if (SoundClip->GetChannelsCount() == 1)
								{
									Frames[i].L = Frames[i].R = *(SoundClip->GetBuffer() + Offset++);
								}
								else
								{
									Frames[i].L = *(SoundClip->GetBuffer() + Offset++);
									Frames[i].R = *(SoundClip->GetBuffer() + Offset++);
								}

								Played += TimeStep;
							}

							if (Offset >= SoundClip->GetBufferSize() / sizeof(int16) - Count * sizeof(Sound::Frame))
							{
								Offset = 0;
								Played = 0.0;

								if (!Looping)
								{
									Playing = false;
								}
							}
						}
					}
				}
			}
		}
	}

	AudioSource::~AudioSource() {}

}











