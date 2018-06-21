#pragma once

#include <Audio/AudioDevice.h>
#include <Audio/AudioSource.h>
#include <Audio/AudioListener.h>
#include <Common/Sound/Sound.h>
#include <Core/Containers/Array.h>
#include <Core/Memory.h>

namespace Columbus
{

	class AudioMixer
	{
	private:
		AudioListener Listener;
		Array<AudioSource*> Sources;

		Sound::Frame* Data = nullptr;
		Sound::FrameHight* Mixed = nullptr;

		bool BufferInitialized = false;
	public:
		AudioMixer() {}

		void AddSource(AudioSource* Source)
		{
			Sources.Add(Source);
		}

		void SetListener(AudioListener InListener)
		{
			Listener = InListener;
		}

		void Update(Sound::Frame* Frames, uint32 Count);

		~AudioMixer()
		{
			delete[] Data;
			delete[] Mixed;
		}
	};

}






