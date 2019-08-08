#include <Audio/AudioSource.h>
#include <Common/Sound/SoundUtil.h>
#include <cstring>

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
		SoundClip = nullptr;
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
			Offset = static_cast<uint64>(Time * SoundClip->GetFrequency() * SoundClip->GetChannelsCount());
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
		if (Playing && SoundClip != nullptr)
		{
			uint32 Decoded = SoundClip->Decode(Frames, Count, Offset);

			Played += 1.0 / SoundClip->GetFrequency() * SoundClip->GetChannelsCount() * Count;

			if (Decoded < Count)
			{
				SoundClip->Seek(0);
				Offset = 0;
				Played = 0.0;
			}
		} else
		{
			memset(Frames, 0, Count * sizeof(Sound::Frame));
		}
	}

	AudioSource::~AudioSource() {}

}


