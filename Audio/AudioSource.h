#pragma once

#include <Common/Sound/Sound.h>
#include <Math/Vector3.h>

namespace Columbus
{

	class AudioSource
	{
	public:
		enum class Mode
		{
			Sound2D,
			Sound3D
		};
	protected:
		Sound* SoundClip;
		uint64 Offset;
		double Played;
	public:
		Vector3 Position;
		Vector3 Velocity;
		Vector3 Direction;
		float Gain;
		float Pitch;
		float MinDistance;
		float MaxDistance;
		float Rolloff;
		bool Playing;
		bool Looping;
		Mode SoundMode;
	public:
		AudioSource();

		void Play();
		void Pause();
		void Stop();
		void Rewind();

		void SetPlayedTime(double Time);
		double GetPlayedTime() const;

		void SetSound(Sound* InSound);
		Sound* GetSound() const;
		void PrepareBuffer(Sound::Frame* Frames, uint32 Count);

		~AudioSource();
	};

}









