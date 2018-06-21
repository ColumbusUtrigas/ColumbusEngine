#include <Audio/AudioMixer.h>
#include <Math/Vector2.h>

namespace Columbus
{

	static float CalculateSourceAttenuation(AudioSource* Source, Vector3 Listener)
	{
		float Attenuation = 1.0f;

		if (Source->GetMode() == AudioSource::Mode::Sound3D)
		{
			float Distance = Source->GetPosition().Length(Listener);
			float MinDist = Source->GetMinDistance();
			float MaxDist = Source->GetMaxDistance();
			float Rolloff = Source->GetRolloff();

			if (Distance == 0.0f)
			{
				Distance = 0.000001f;
			}

			if (Distance < MaxDist)
			{
				Attenuation = Math::Clamp(MinDist / (MinDist + (Rolloff * (Distance - MinDist))), 0.0f, 1.0f);
			}
			else
			{
				Attenuation = 0.0f;
			}
		}

		return Attenuation;
	}

	static float CalculateSourcePan(AudioSource* Source, AudioListener Listener)
	{
		if (Source != nullptr)
		{
			if (Source->GetMode() == AudioSource::Mode::Sound3D)
			{
				return Vector3::Dot(Vector3::Normalize(Vector3::Cross(Listener.Forward, Listener.Up)), Vector3::Normalize(Source->GetPosition() - Listener.Position));
			}
		}

		return 0.0f;
	}

	static void AudioBufferClip(Sound::FrameHight* Buffer, uint32 Count)
	{
		if (Buffer != nullptr)
		{
			for (uint32 i = 0; i < Count; i++)
			{
				if (Buffer[i].L > 0x7FFF)
				{
					Buffer[i].L = 0x7FFF;
				}
				else if (Buffer[i].L < -0x7FFF)
				{
					Buffer[i].L = -0x7FFF;
				}

				if (Buffer[i].R > 0x7FFF)
				{
					Buffer[i].R = 0x7FFF;
				}
				else if (Buffer[i].R < -0x7FFF)
				{
					Buffer[i].R = -0x7FFF;
				}
			}
		}
	}

	void AudioMixer::Update(Sound::Frame* Frames, uint32 Count)
	{
		if (!BufferInitialized)
		{
			Data = new Sound::Frame[Count];
			Mixed = new Sound::FrameHight[Count];

			BufferInitialized = true;
		}

		Memory::Memset(Data, 0, Count * sizeof(Sound::Frame));
		Memory::Memset(Mixed, 0, Count * sizeof(Sound::FrameHight));

		for (auto& Source : Sources)
		{
			float Attenuation = CalculateSourceAttenuation(Source, Listener.Position);
			float Pan = CalculateSourcePan(Source, Listener);

			float LVolume = Math::Min(1.0f, 1.0f - Pan);
			float RVolume = Math::Min(1.0f, 1.0f + Pan);

			float Gain = Source->GetGain() * Attenuation;
			Gain = 1.0f - Math::Sqrt(1.0f - Gain * Gain);

			Source->PrepareBuffer(Data, Count);

			for (uint32 i = 0; i < Count; i++)
			{
				Mixed[i].L += Data[i].L * Gain * LVolume;
				Mixed[i].R += Data[i].R * Gain * RVolume;
			}
		}

		AudioBufferClip(Mixed, Count);

		for (uint32 i = 0; i < Count; i++)
		{
			Frames[i].L = (int16)Mixed[i].L;
			Frames[i].R = (int16)Mixed[i].R;
		}
	}

}























