#pragma once

#include <Common/Sound/Sound.h>
#include <map>

#include <SDL.h>
#include <iostream>

namespace Columbus
{

	static Uint8* AudioPos;
	static Uint32 AudioLen;
	static Uint32 AudioLength;

	void audioCallback(void* userdata, Uint8* stream, int len)
	{
		if (AudioLen == 0)
		{
			AudioPos -= AudioLength;
			AudioLen = AudioLength;
			return;
		}
		len = (len > AudioLen ? AudioLen : len);
		SDL_memcpy(stream, AudioPos, len);
		AudioPos += len;
		AudioLen -= len;
	}

	class AudioPlayer
	{
	protected:
		std::map<uint32, Sound*> SoundClips;
	public:
		Sound* Clip; //Just testing
		SDL_AudioSpec Spec;

		AudioPlayer()
		{
			Clip = new Sound();
			Clip->Load("Data/Sounds/cartoon001.ogg");

			Spec.freq = Clip->GetFrequency();
			Spec.format = AUDIO_S16;
			Spec.channels = Clip->GetChannelsCount();
			Spec.samples = Clip->GetBufferSize() / Clip->GetChannelsCount() / sizeof(uint16);
			Spec.callback = audioCallback;
			Spec.userdata = nullptr;

			AudioPos = (Uint8*)Clip->GetBuffer();
			AudioLen = Clip->GetBufferSize();
			AudioLength = Clip->GetBufferSize();

			if (SDL_OpenAudio(&Spec, NULL) < 0) std::cout << SDL_GetError() << std::endl;
		}

		void AddSoundClip(uint32 Key, Sound* SoundClip)
		{
			if (SoundClip != nullptr)
			{
				SoundClips[Key] = SoundClip;
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

		virtual ~AudioPlayer() {}
	};

}




