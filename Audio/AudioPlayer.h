#pragma once

#include <Common/Sound/Sound.h>
#include <Audio/AudioMixer.h>
#include <System/Log.h>
#include <System/Assert.h>
#include <SDL.h>

namespace Columbus
{

	static void audioCallback(void* UserData, Uint8* Stream, int StreamSize)
	{
		Memory::Memset(Stream, 0, StreamSize);
		
		if (UserData != nullptr)
		{
			static_cast<AudioMixer*>(UserData)->Update((Sound::Frame*)Stream, StreamSize / sizeof(Sound::Frame));
		}
	}

	class AudioPlayer
	{
	private:
		SDL_AudioSpec Spec;
	public:
		AudioPlayer(uint16 Channels, uint32 Frequency, AudioMixer* Mixer)
		{
			Log::initialization("Audio system initialization");
			COLUMBUS_ASSERT_MESSAGE(Channels >= 1, "AudioPlayer::AudioPlayer(): Invalid channels count")
			COLUMBUS_ASSERT_MESSAGE(Frequency > 0, "AudioPlayer::AudioPlayer(): Invalid frequency")
			COLUMBUS_ASSERT_MESSAGE(Mixer, "AudioPlayer::AudioPlayer(): Invalid mixer")

			Spec.freq = Frequency;
			Spec.format = AUDIO_S16;
			Spec.channels = Channels;
			Spec.userdata = Mixer;
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

			if (SDL_OpenAudio(&Spec, NULL) < 0)
			{
				printf("%s\n", SDL_GetError());
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
				Log::error("Couldn't find audio recording device");
			}

			Log::initialization("Audio drivers count: " + std::to_string(AudioDriversCount));

			for (uint32 i = 0; i < AudioDriversCount; i++)
			{
				Log::initialization("Audio driver (" + std::to_string(i + 1) + "): " + SDL_GetAudioDriver(i));
			}

			Log::initialization("Current audio driver: " + std::string(SDL_GetCurrentAudioDriver()));
			Log::initialization("Audio playback devices count: " + std::to_string(AudioPlaybackDevicesCount));

			for (uint32 i = 0; i < AudioPlaybackDevicesCount; i++)
			{
				if (SDL_GetAudioDeviceName(i, 0) != nullptr)
				{
					Log::initialization("Audio playback device name (" + std::to_string(i + 1) + "): " + SDL_GetAudioDeviceName(i, 0));
				}
				else
				{
					Log::initialization("Audio playback device name (" + std::to_string(i + 1) + "): " + "");
				}
			}

			Log::initialization("Audio recording devices count: " + std::to_string(AudioRecordingDevicesCount));

			for (uint32 i = 0; i < AudioRecordingDevicesCount; i++)
			{
				if (SDL_GetAudioDeviceName(i, 1) != nullptr)
				{
					Log::initialization("Audio recording device name (" + std::to_string(i + 1) + "): "  + SDL_GetAudioDeviceName(i, 1));
				}
				else
				{
					Log::initialization("Audio recording device name (" + std::to_string(i + 1) + "): " + "");
				}
			}

			Log::initialization("Audio system initialized\n");
		}

		void Play()
		{
			SDL_PauseAudio(0);
		}

		void Stop()
		{
			SDL_PauseAudio(1);
		}

		virtual ~AudioPlayer()
		{
			//SDL_PauseAudio(1);
			//SDL_LockAudio();
			//SDL_CloseAudio();
		}
	};

}




