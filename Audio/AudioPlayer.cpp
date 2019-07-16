#include <Audio/AudioPlayer.h>
#include <System/Log.h>
#include <Core/Assert.h>
#include <SDL.h>
#include <cstring>

namespace Columbus
{

	static void AudioCallback(void* UserData, Uint8* Stream, int StreamSize)
	{
		if (UserData != nullptr)
		{
			float Speed = ((AudioMixer*)(UserData))->GetSpeed();

			if (Speed != 0.0f)
			{
				uint32 Size = StreamSize / sizeof(Sound::Frame);
				static Sound::Frame* Frames = new Sound::Frame[Size];
				Sound::Frame* Data = (Sound::Frame*)Stream;

				if (Speed == 1.0f)
				{
					((AudioMixer*)(UserData))->Update((Sound::Frame*)Stream, StreamSize / sizeof(Sound::Frame));
					return;
				}

				((AudioMixer*)(UserData))->Update(Frames, Size * Speed);

				float Step = 1.0 / Speed;

				for (uint32 i = 0; i < Size; i++)
				{
					uint32 Left = (uint32)(i * Speed);
					uint32 Right = (uint32)(i * Speed + 1);
					float S = fmodf(Step * i, Speed);
					Data[i].L = (int32)(Math::Mix((float)Frames[Left].L, (float)Frames[Right].L, S));
					Data[i].R = (int32)(Math::Mix((float)Frames[Left].R, (float)Frames[Right].R, S));
				}

				return;
			}
		}
		
		memset(Stream, 0, StreamSize);
	}

	AudioPlayer::AudioPlayer(uint16 Channels, uint32 Frequency, AudioMixer* Mixer)
	{
		Log::Initialization("Audio system initialization");
		COLUMBUS_ASSERT_MESSAGE(Channels >= 1, "AudioPlayer::AudioPlayer(): Invalid channels count")
		COLUMBUS_ASSERT_MESSAGE(Frequency > 0, "AudioPlayer::AudioPlayer(): Invalid frequency")
		COLUMBUS_ASSERT_MESSAGE(Mixer, "AudioPlayer::AudioPlayer(): Invalid mixer")

		SDL_AudioSpec Spec;

		Spec.freq = Frequency;
		Spec.format = AUDIO_S16;
		Spec.channels = (Uint8)Channels;
		Spec.userdata = Mixer;
		Spec.callback = AudioCallback;

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

		if (SDL_OpenAudio(&Spec, NULL) < 0)
		{
			Log::Error("%s\n", SDL_GetError());
			COLUMBUS_ASSERT_MESSAGE(0, "AudioPlayer::AudioPlayer(): Couldn't initialize audio stream")
		}

		uint32 AudioDriversCount = SDL_GetNumAudioDrivers();
		uint32 AudioPlaybackDevicesCount = SDL_GetNumAudioDevices(0);
		uint32 AudioRecordingDevicesCount = SDL_GetNumAudioDevices(1);

		if (AudioDriversCount == 0)
		{
			COLUMBUS_ASSERT_MESSAGE(0, "Couldn't find audio driver")
		}

		if (AudioPlaybackDevicesCount == 0)
		{
			COLUMBUS_ASSERT_MESSAGE(0, "Couldn't find audio playback device")
		}

		if (AudioRecordingDevicesCount == 0)
		{
			Log::Error("Couldn't find audio recording device");
		}

		Log::Initialization("Audio drivers count: %i", AudioDriversCount);

		for (uint32 i = 0; i < AudioDriversCount; i++)
		{
			Log::Initialization("Audio driver (%i): %s", i + 1, SDL_GetAudioDriver(i));
		}

		Log::Initialization("Current audio driver: %s", SDL_GetCurrentAudioDriver());
		Log::Initialization("Audio playback devices count: %i", AudioPlaybackDevicesCount);

		for (uint32 i = 0; i < AudioPlaybackDevicesCount; i++)
		{
			const char* Name = SDL_GetAudioDeviceName(i, 0);
			Log::Initialization("Audio playback device name (%i): %s", i + 1, Name != nullptr ? Name : "");
		}

		Log::Initialization("Audio recording devices count: %i", AudioRecordingDevicesCount);

		for (uint32 i = 0; i < AudioRecordingDevicesCount; i++)
		{
			const char* Name = SDL_GetAudioDeviceName(i, 1);
			Log::Initialization("Audio recording device name (%i): %s", i + 1, Name != nullptr ? Name : "");
		}

		Log::Initialization("Audio system initialized\n");
	}

	void AudioPlayer::Play()
	{
		SDL_PauseAudio(0);
	}

	void AudioPlayer::Stop()
	{
		SDL_PauseAudio(1);
	}

}


