#pragma once

#include <Common/Sound/Sound.h>
#include <SDL.h>

namespace Columbus
{

	static Uint8* AudioPos;
	static Uint32 AudioLen;
	static Uint32 AudioLength;

	static void audioCallback(void* userdata, Uint8* stream, int len)
	{
		if (AudioLen == 0)
		{
			AudioPos -= AudioLength;
			AudioLen = AudioLength;
			return;
		}

		len = (len > AudioLen ? AudioLen : len);

		Memory::Memcpy(stream, AudioPos, len);
		AudioPos += len;
		AudioLen -= len;
	}

	class AudioPlayer
	{
	private:
		SDL_AudioSpec Spec;
	public:
		AudioPlayer(const uint16* Data, uint16 Channels, uint32 Frequency, uint64 Size)
		{
			COLUMBUS_ASSERT_MESSAGE(Data, "AudioPlayer::AudioPlayer(): Invalid data")
			COLUMBUS_ASSERT_MESSAGE(Channels >= 1, "AudioPlayer::AudioPlayer(): Invalid channels count")
			COLUMBUS_ASSERT_MESSAGE(Frequency > 0, "AudioPlayer::AudioPlayer(): Invalid frequency")
			COLUMBUS_ASSERT_MESSAGE(Size > 0, "AudioPlayer::AudioPlayer(): Invalid size")

			Spec.freq = Frequency;
			Spec.format = AUDIO_S16;
			Spec.channels = Channels;
			Spec.callback = audioCallback;

			if (Spec.freq <= 11025)
			{
				Spec.samples = 256;
			}
			else if (Spec.freq <= 22050)
			{
				Spec.samples = 512;
			}
			else if (Spec.freq <= 44100)
			{
				Spec.samples = 1024;
			}
			else
			{
				Spec.samples = 2048;
			}

			AudioPos = (Uint8*)Data;
			AudioLen = Size;
			AudioLength = Size;

			if (SDL_OpenAudio(&Spec, NULL) < 0)
			{
				printf("%s\n", SDL_GetError());
				COLUMBUS_ASSERT_MESSAGE(0, "AudioPlayer::AudioPlayer(): Couldn't initialize audio stream")
			}
		}

		void Play()
		{
			SDL_PauseAudio(0);
		}

		void Pause()
		{
			SDL_PauseAudio(1);
		}

		virtual ~AudioPlayer()
		{
			SDL_CloseAudio();
		}
	};

}



