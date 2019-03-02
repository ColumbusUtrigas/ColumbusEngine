#include <Audio/AudioSource.h>
#include <Core/Memory.h>

namespace Columbus
{

	AudioSource::AudioSource() :
		SoundClip(nullptr),
		Offset(0),
		Played(0.0),
		Position(0.0f),
		Velocity(0.0f),
		Direction(0.0f),
		Gain(1.0f),
		Pitch(1.0f),
		MinDistance(Math::Sqrt(8)),
		MaxDistance(100.0f),
		Rolloff(1.0f),
		Playing(false),
		Looping(false),
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
			Offset = (uint64)(Time * SoundClip->GetFrequency());
			SoundClip->Seek(Offset);
		}
	}

	double AudioSource::GetPlayedTime() const
	{
		return Played;
	}

	void AudioSource::SetSound(Sound* InSound)
	{
		SoundClip = InSound;
	}

	Sound* AudioSource::GetSound() const
	{
		return SoundClip;
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
							uint32 Decoded = SoundClip->Decode(Frames, Count);

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

	AudioSource::~AudioSource() { delete SoundClip; }

}


