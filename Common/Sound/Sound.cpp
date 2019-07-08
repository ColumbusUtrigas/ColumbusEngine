#include <Common/Sound/Sound.h>
#include <Common/Sound/OGG/SoundOGG.h>
#include <Common/Sound/WAV/SoundWAV.h>
#include <System/Assert.h>
#include <cstdlib>
#include <cstring>

namespace Columbus
{

	SoundFormat SoundGetFormat(const char* FileName)
	{
		if (SoundDecoderPCM::IsWAV  (FileName)) return SoundFormat::WAV_PCM;
		if (SoundDecoderADPCM::IsWAV(FileName)) return SoundFormat::WAV_ADPCM;
		if (SoundDecoderOGG::IsOGG  (FileName)) return SoundFormat::OGG;
		if (SoundIsMP3              (FileName)) return SoundFormat::MP3;

		return SoundFormat::Unknown;
	}

	int16* SoundLoad(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		SoundFormat Format = SoundGetFormat(FileName);

		switch (Format)
		{
		case SoundFormat::Unknown:   return nullptr; break;
		case SoundFormat::WAV_PCM:
		case SoundFormat::WAV_ADPCM: return SoundLoadWAV(FileName, OutSize, OutFrequency, OutChannels); break;
		case SoundFormat::OGG:       return SoundLoadOGG(FileName, OutSize, OutFrequency, OutChannels); break;
		case SoundFormat::MP3:       return SoundLoadMP3(FileName, OutSize, OutFrequency, OutChannels); break;
		}

		return nullptr;
	}
	/*
	* Sound class
	*/
	Sound::Sound() :
		BufferSize(0),
		Frequency(0),
		Channels(0),
		Buffer(nullptr),
		Streaming(false),
		Decoder(nullptr) {}
	
	bool Sound::Load(const char* FileName, bool Stream)
	{
		Free();

		if (Stream == false)
		{
			Buffer = SoundLoad(FileName, BufferSize, Frequency, Channels);
			Streaming = false;
			return Buffer != nullptr;
		}
		else
		{
			switch (SoundGetFormat(FileName))
			{
			case SoundFormat::WAV_PCM:
				Decoder = new SoundDecoderPCM();
				Streaming = true;
				return Decoder->Load(FileName);
			case SoundFormat::WAV_ADPCM:
				Decoder = new SoundDecoderADPCM();
				Streaming = true;
				return Decoder->Load(FileName);
			case SoundFormat::OGG:
				Decoder = new SoundDecoderOGG();
				Streaming = true;
				return Decoder->Load(FileName);
				break;
			case SoundFormat::MP3:
				Streaming = false;
				return false;
				break;
			case SoundFormat::Unknown:
				Streaming = false;
				return false;
				break;
			}
		}

		return false;
	}

	void Sound::Free()
	{
		BufferSize = 0;
		Frequency = 0;
		Channels = 0;
		//delete[] Buffer;
		if (Buffer != nullptr) free(Buffer);
		Streaming = false;
		if (Decoder != nullptr) delete Decoder;
		Decoder = nullptr;
	}

	void Sound::Seek(uint64 Offset)
	{
		if (Streaming)
		{
			if (Decoder != nullptr)
			{
				Decoder->Seek(Offset);
			}
		}
	}

	static void ResampleAudio(Sound::Frame* src, Sound::Frame* dst, int from_samples, int from_freq, int to_freq)
	{
		if (from_freq == to_freq)
		{
			// so, there is nothing to do, just copy samples
			memcpy(dst, src, from_samples * sizeof(Sound::Frame));
			return;
		}

		if (from_freq * 2 == to_freq)
		{
			// 11025->22050 or 22050 to 44100
			for (int i = 0; i < from_samples; i++)
			{
				dst[i * 2 + 0] = src[i];
				dst[i * 2 + 1] = src[i];
			}

			return;
		}

		if (from_freq * 4 == to_freq)
		{
			// 11025->44100
			for (int i = 0; i < from_samples; i++)
			{
				dst[i * 4 + 0] = src[i];
				dst[i * 4 + 1] = src[i];
				dst[i * 4 + 2] = src[i];
				dst[i * 4 + 3] = src[i];
			}

			return;
		}

		COLUMBUS_ASSERT_MESSAGE(false, "Sound module: ResampleAudio(): invalid conversion");
	}

	uint32 Sound::Decode(Frame* Frames, uint32 Count)
	{
		if (Frames != nullptr)
		{
			if (Streaming)
			{
				if (Decoder != nullptr)
				{
					static constexpr int MaxSamples = 2048;

					uint32 Divider = 44100 / Decoder->GetFrequency();
					Frame Tmp[MaxSamples];

					int from_freq = Decoder->GetFrequency();
					int to_freq = 44100;

					uint32 Decoded = Decoder->Decode(Tmp, Count / Divider);
					ResampleAudio(Tmp, Frames, Decoded, from_freq, to_freq);

					return Decoded * Divider;
				}
			}
		}

		return 0;
	}

	uint64 Sound::GetBufferSize() const
	{
		if (Streaming)
		{
			if (Decoder != nullptr)
			{
				return Decoder->GetSize();
			}
		}
		else
		{
			return BufferSize;
		}

		return 0;
	}

	uint32 Sound::GetFrequency() const
	{
		if (Streaming)
		{
			if (Decoder != nullptr)
			{
				return Decoder->GetFrequency();
			}
		}
		else
		{
			return Frequency;
		}

		return 0;
	}

	uint16 Sound::GetChannelsCount() const
	{
		if (Streaming)
		{
			if (Decoder != nullptr)
			{
				return Decoder->GetChannels();
			}
		}
		else
		{
			return Channels;
		}
		
		return 0;
	}

	int16* Sound::GetBuffer() const
	{
		return Buffer;
	}

	double Sound::GetLength() const
	{
		return (double)BufferSize / (double)Frequency / (double)(Channels * sizeof(int16));
	}

	bool Sound::IsStreaming() const
	{
		return Streaming && Decoder != nullptr;
	}

	Sound::~Sound()
	{
		Free();
	}

}









