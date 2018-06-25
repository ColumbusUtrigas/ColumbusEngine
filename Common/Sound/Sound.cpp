#include <Common/Sound/Sound.h>
#include <Core/Memory.h>

namespace Columbus
{

	SoundFormat SoundGetFormat(std::string FileName)
	{
		if (SoundIsWAV_PCM(FileName)) return SoundFormat::WAV_PCM;
		if (SoundIsWAV_ADPCM(FileName)) return SoundFormat::WAV_ADPCM;
		if (SoundIsOGG(FileName)) return SoundFormat::OGG;
		if (SoundIsMP3(FileName)) return SoundFormat::MP3;

		return SoundFormat::Unknown;
	}

	int16* SoundLoad(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
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
		Decoder(nullptr)
	{ }
	
	bool Sound::Load(std::string FileName, bool Stream)
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
		if (Buffer != nullptr) Memory::Free(Buffer);
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

	uint32 Sound::Decode(Frame* Frames, uint32 Count)
	{
		if (Frames != nullptr)
		{
			if (Streaming)
			{
				if (Decoder != nullptr)
				{
					return Decoder->Decode(Frames, Count);
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









