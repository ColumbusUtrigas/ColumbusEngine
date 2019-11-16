#include <Audio/AudioMixer.h>
#include <Common/Sound/Sound.h>
#include <Profiling/Profiling.h>
#include <cstring>
#include <cstdio>

namespace Columbus
{

	static float CalculateSourceAttenuation(AudioSource* Source, const Vector3& Listener)
	{
		float Attenuation = 1.0f;

		if (Source->SoundMode == AudioSource::Mode::Sound3D)
		{
			float Distance = Math::Max(0.000001f, Source->Position.Length(Listener));
			float MinDist = Source->MinDistance;
			float MaxDist = Source->MaxDistance;
			float Rolloff = Source->Rolloff;

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

	static float CalculateSourcePan(AudioSource* Source, const AudioListener& Listener)
	{
		if (Source->SoundMode == AudioSource::Mode::Sound3D)
		{
			return Vector3::Dot(Vector3::Normalize(Vector3::Cross(Listener.Forward, Listener.Up)), Vector3::Normalize(Source->Position - Listener.Position));
		}

		return 0.0f;
	}

	static void AudioBufferClip(Sound::FrameHight* Buffer, uint32 Count)
	{
		for (uint32 i = 0; i < Count; i++)
		{
			Buffer[i].L = Math::Clamp(Buffer[i].L, -0x7FFF, 0x7FFF);
			Buffer[i].R = Math::Clamp(Buffer[i].R, -0x7FFF, 0x7FFF);
		}
	}

	void AudioMixer::Update(Sound::Frame* Frames, uint32 Count)
	{
		PROFILE_CPU(ProfileModule::Audio);

		if (!BufferInitialized)
		{
			Data = new Sound::Frame[Count];
			Mixed = new Sound::FrameHight[Count];

			BufferInitialized = true;
		}
		
		memset(Mixed, 0, Count * sizeof(Sound::FrameHight));

		for (auto Source : Sources)
		{
			if ((bool)Source)
			{
				float Attenuation = CalculateSourceAttenuation(Source.get(), Listener.Position);
				float Pan = CalculateSourcePan(Source.get(), Listener);

				float LVolume = Math::Min(1.0f, 1.0f - Pan);
				float RVolume = Math::Min(1.0f, 1.0f + Pan);

				float Gain = Source->Gain * Attenuation;
				Gain = 1.0f - Math::Sqrt(1.0f - Gain * Gain);

				Source->PrepareBuffer(Data, Count);

				for (uint32 i = 0; i < Count; i++)
				{
					Mixed[i].L += static_cast<int32>(Data[i].L * Gain * LVolume);
					Mixed[i].R += static_cast<int32>(Data[i].R * Gain * RVolume);
				}
			}
		}

		AudioBufferClip(Mixed, Count);

		for (uint32 i = 0; i < Count; i++)
		{
			Frames[i].L = static_cast<int16>(Mixed[i].L);
			Frames[i].R = static_cast<int16>(Mixed[i].R);
		}
	}

}


