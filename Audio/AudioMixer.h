#pragma once

#include <Audio/AudioSource.h>
#include <Audio/AudioListener.h>
#include <Core/Containers/Vector.h>

namespace Columbus
{

	class AudioMixer
	{
	private:
		AudioListener Listener;
		Vector<AudioSource*> Sources;

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

		bool HasSource(AudioSource* Source)
		{
			for (const auto& Audio : Sources)
			{
				if (Audio == Source)
				{
					return true;
				}
			}

			return false;
		}

		void Update(Sound::Frame* Frames, uint32 Count);

		~AudioMixer()
		{
			delete[] Data;
			delete[] Mixed;
		}
	};

}






