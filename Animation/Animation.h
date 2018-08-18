#pragma once

#include <Math/Vector3.h>
#include <Math/InterpolationCurve.h>

namespace Columbus
{

	class Animation
	{
	protected:
		Vector3 CurrentPosition;
	public:
		float Played;
		float Speed;
		float Length;
		bool Loop;
		bool Playing;

		InterpolationCurve<Vector3> PositionAnimation;
	public:
		Animation() : Played(0.0f), Speed(1.0f), Length(1.0f), Loop(true), Playing(true) {}

		Vector3 GetCurrentPosition() const { return CurrentPosition; }

		void Play() { Playing = true;  }
		void Stop() { Playing = false; }
		void Rewind() { Played = 0.0f; }

		void SetPlayed(float InPlayed) { Played = InPlayed; }
		void SetSpeed(float InSpeed) { Speed = InSpeed; }
		void SetLength(float InLength) { Length = InLength; }
		void SetLoop(bool InLoop) { Loop = InLoop; }

		void Update(float TimeTick)
		{
			if (Playing)
			{
				Played += TimeTick * Speed;

				if (Played > Length && Loop) Played = 0.0f;
				if (Played < 0 && Loop) Played = Length;

				CurrentPosition = PositionAnimation.Interpolate(Played);
			}
		}

		~Animation() {}
	};

}













