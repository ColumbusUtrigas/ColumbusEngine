#pragma once

#include <Audio/AudioSource.h>

namespace Columbus
{

	class AudioSourceOpenAL : public AudioSource
	{
	private:
		uint32 OpenALBuffer;
		uint32 OpenALSource;
	public:
		AudioSourceOpenAL();

		void Play() override;
		void Pause() override;
		void Stop() override;
		void Rewind() override;

		void SetSound(Sound* InSound) override;
		void SetPosition(Vector3 InPosition) override;
		void SetVelocity(Vector3 InVelocity) override;
		void SetDirection(Vector3 InDirection) override;
		void SetGain(float InGain) override;
		void SetPitch(float InPitch) override;
		void SetLooping(bool InLooping) override;

		~AudioSourceOpenAL() override;
	};

}












