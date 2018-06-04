#pragma once

#include <Audio/AudioDevice.h>
#include <Audio/AudioSource.h>
#include <Common/Sound/Sound.h>
#include <Core/Memory.h>

namespace Columbus
{

	class AudioMixer
	{
	private:

	public:
		AudioSource* Source;
		Sound Clip;
		uint64 Offset = 0;
	public:
		AudioMixer()
		{
			Source = new AudioSource();
			Source->GetSound()->Load("Data/Sounds/thestonemasons.ogg");
			//Clip.Load("Data/Sounds/thestonemasons.ogg");
			Source->Play();
			Source->SetLooping(true);
		}

		void Update(Sound::Frame* Frames, uint32 Count)
		{
			Source->PrepareBuffer(Frames, Count);
			/*Memory::Memset(OutBuffer, 0, Size);

			if (Offset >= Clip.GetBufferSize() / sizeof(int16) - Size)
			{
				uint32 BufferSize = Offset - (Clip.GetBufferSize() / sizeof(int16) - Size);
				Memory::Memcpy(OutBuffer, Clip.GetBuffer() + Offset, BufferSize);
				Offset = 0;
			}
			else
			{
				Memory::Memcpy(OutBuffer, Clip.GetBuffer() + Offset, Size);
				Offset += Size / sizeof(int16);
			}*/
		}

		~AudioMixer() {}
	};

}






