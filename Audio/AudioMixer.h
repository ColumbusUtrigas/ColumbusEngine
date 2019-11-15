#pragma once

#include <Audio/AudioSource.h>
#include <Audio/AudioListener.h>
#include <vector>
#include <memory>

namespace Columbus
{

	class AudioMixer
	{
	private:
		using SourcesList = std::vector<std::shared_ptr<AudioSource>>;

		friend class Renderer;
		
		AudioListener Listener;
		SourcesList Sources;

		Sound::Frame* Data = nullptr;
		Sound::FrameHight* Mixed = nullptr;

		float Speed = 1.0f;
		bool BufferInitialized = false;
	public:
		AudioMixer() {}

		void Clear()
		{
			Sources.clear();
		}

		void AddSource(std::shared_ptr<AudioSource> Source)
		{
			Sources.push_back(Source);
		}

		void SetListener(AudioListener InListener)
		{
			Listener = InListener;
		}

		void SetSpeed(float InSpeed) { Speed = InSpeed; }
		float GetSpeed() const { return Speed; }

		void Update(Sound::Frame* Frames, uint32 Count);

		~AudioMixer()
		{
			delete[] Data;
			delete[] Mixed;
		}
	};

}


