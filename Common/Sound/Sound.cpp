#include <Common/Sound/Sound.h>
#include <Common/Sound/OGG/SoundOGG.h>
#include <Common/Sound/WAV/SoundWAV.h>
#include <Common/Sound/SoundUtil.h>
#include <cstdlib>

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
		case SoundFormat::Unknown:   return nullptr;
		case SoundFormat::WAV_PCM:
		case SoundFormat::WAV_ADPCM: return SoundLoadWAV(FileName, OutSize, OutFrequency, OutChannels);
		case SoundFormat::OGG:       return SoundLoadOGG(FileName, OutSize, OutFrequency, OutChannels);
		case SoundFormat::MP3:       return SoundLoadMP3(FileName, OutSize, OutFrequency, OutChannels);
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
			case SoundFormat::MP3:
				Streaming = false;
				return false;
			case SoundFormat::Unknown:
				Streaming = false;
				return false;
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

	uint32 Sound::Decode(Frame* Frames, uint32 Count, uint64& Offset)
	{
		if (Frames != nullptr)
		{
			static constexpr int MaxSamples = 2048;
			uint32 Divider = 44100 / GetFrequency();
			Frame Tmp[MaxSamples];

			int from_freq = GetFrequency();
			int to_freq = 44100;

			if (Streaming)
			{
				if (Decoder != nullptr)
				{
					uint32 Decoded = Decoder->Decode(Tmp, Count / Divider);
					SoundUtil::Resample(Tmp, Frames, Decoded, from_freq, to_freq);

					Offset += Count * Channels;

					return Decoded * Divider;
				}
			} else
			{
				uint32 RealCount = Count / Divider;

				if (Channels == 1)
				{
					for (uint32 i = 0; i < RealCount; i++)
					{
						Tmp[i].L = Tmp[i].R = Buffer[Offset++ % BufferSize];
					}
				} else if (Channels == 2)
				{
					for (uint32 i = 0; i < RealCount; i++)
					{
						Tmp[i].L = Buffer[Offset++ % BufferSize];
						Tmp[i].R = Buffer[Offset++ % BufferSize];
					}
				}

				SoundUtil::Resample(Tmp, Frames, RealCount, from_freq, to_freq);

				return Count;
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


