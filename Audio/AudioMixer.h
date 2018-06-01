#pragma once

#include <Audio/AudioPlayer.h>

namespace Columbus
{

	class AudioMixer
	{
	private:

	public:
		float Gain = 1.0f;
		Sound Clip;
		AudioPlayer* Player;
		int16* Data;
		uint64 Offset = 0;
		uint32 Samples = 0;
	public:
		AudioMixer()
		{
			Clip.Load("Data/Sounds/thestonemasons.ogg");

			if (Clip.GetFrequency() <= 11025)
			{
				Data = new int16[256 * Clip.GetChannelsCount()];
				Samples = 256;
			}
			else if (Clip.GetFrequency() <= 22050)
			{
				Data = new int16[512 * Clip.GetChannelsCount()];
				Samples = 512;
			}
			else if (Clip.GetFrequency() <= 44100)
			{
				Data = new int16[1024 * Clip.GetChannelsCount()];
				Samples = 1024;
			}
			else
			{
				Data = new int16[2048 * Clip.GetChannelsCount()];
				Samples = 2048;
			}

			Player = new AudioPlayer(Data, Clip.GetChannelsCount(), Clip.GetFrequency(), Samples * Clip.GetChannelsCount() * sizeof(int16));
		}

		void Play()
		{
			Player->Play();
		}

		void Update(const float TimeTick)
		{
			if (TimeToSet)
			{
				if (Offset >= Clip.GetBufferSize() / sizeof(int16) - Samples * Clip.GetChannelsCount())
				{
					Offset = 0;
				}

				Memory::Memcpy(Data, Clip.GetBuffer() + Offset, Samples * Clip.GetChannelsCount() * 2);
				Player->SetData(Data);
				TimeToSet = false;
				Offset += Samples * Clip.GetChannelsCount();
			}
		}

		~AudioMixer()
		{
			delete[] Data;
		}
	};

}






