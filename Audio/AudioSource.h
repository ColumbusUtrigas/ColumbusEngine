#pragma once

#include <Math/Vector3.h>
#include <Common/Sound/Sound.h>

namespace Columbus
{

	class AudioSource
	{
	public:
		enum class Mode;
	protected:
		Sound* SoundClip;
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

		uint64 Offset;
		double Played;

		Mode SoundMode;
	public:
		enum class Mode
		{
			Sound2D,
			Sound3D
		};
	public:
		AudioSource();

		virtual void Play();
		virtual void Pause();
		virtual void Stop();
		virtual void Rewind();

		void SetPlayedTime(double Time);
		double GetPlayedTime() const;

		void SetMode(Mode InMode);
		Mode GetMode() const;

		virtual void SetSound(Sound* InSound);
		virtual void SetPosition(Vector3 InPosition);
		virtual void SetVelocity(Vector3 InVelocity);
		virtual void SetDirection(Vector3 InDirection);
		virtual void SetGain(float InGain);
		virtual void SetPitch(float InPitch);
		virtual void SetMinDistance(float InDistance);
		virtual void SetMaxDistance(float InDistance);
		virtual void SetRolloff(float InRollof);
		virtual void SetLooping(bool InLooping);

		virtual Sound* GetSound() const;
		virtual Vector3 GetPosition() const;
		virtual Vector3 GetVelocity() const;
		virtual Vector3 GetDirection() const;
		virtual float GetGain() const;
		virtual float GetPitch() const;
		virtual float GetMinDistance() const;
		virtual float GetMaxDistance() const;
		virtual float GetRolloff() const;
		virtual bool IsPlaying() const;
		virtual bool IsLooping() const;

		virtual void PrepareBuffer(Sound::Frame* Frames, uint32 Count);

		virtual ~AudioSource();
	};

}









