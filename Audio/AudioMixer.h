#pragma once

#include <Audio/AudioSource.h>
#include <Audio/AudioListener.h>
#include <Audio/Filters.h>
#include <algorithm>
#include <vector>
#include <memory>
#include <mutex>

namespace Columbus
{

	class AudioMixer
	{
	private:
		using SourcesList = std::vector<std::shared_ptr<AudioSource>>;

		friend class Renderer;

		std::mutex ThreadAccessMt;
		
		AudioListener Listener;
		SourcesList Sources;

		Sound::Frame* Data = nullptr;
		Sound::FrameHight* Mixed = nullptr;

		float Speed = 1.0f;
		bool BufferInitialized = false;

	public: // filters
		AudioFilterLowPass LowPass;

	public:
		AudioMixer() {}

		void Clear()
		{
			Sources.clear();
		}

		// simpler fire and forget interface

		void PlaySound2D(Sound* Clip);


		// main interface

		void AddSource(std::shared_ptr<AudioSource> Source)
		{
			std::lock_guard lg(ThreadAccessMt);
			Sources.push_back(Source);
		}

		void RemoveSource(std::shared_ptr<AudioSource> Source)
		{
			std::lock_guard lg(ThreadAccessMt);
			Sources.erase(std::remove(Sources.begin(), Sources.end(), Source));
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


