#include <Common/Sound/Sound.h>
#include <Common/Sound/OGG/SoundOGG.h>
#include <Core/Memory.h>
#include <System/File.h>
#include <stb_vorbis.h>

namespace Columbus
{

	int16* SoundLoadOGG(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		int16* Ret;
		OutSize = stb_vorbis_decode_filename(FileName.c_str(), (int*)&OutChannels, (int*)&OutFrequency, &Ret) * OutChannels * sizeof(uint16);
		return Ret;
	}

	struct SoundDecoderOGG::StreamData
	{
		stb_vorbis* Ogg;
		stb_vorbis_alloc Alloc;
	};

	SoundDecoderOGG::SoundDecoderOGG() : Data(nullptr) {}

	bool SoundDecoderOGG::IsOGG(std::string FileName)
	{
		File OGGSoundFile(FileName, "rb");
		if (!OGGSoundFile.IsOpened()) return false;

		uint8 Magic[4];
		OGGSoundFile.Read(Magic, sizeof(Magic), 1);
		OGGSoundFile.Close();

		if (Memory::Memcmp(Magic, "OggS", 4) == 0)
		{
			return true;
		}

		return false;
	}

	bool SoundDecoderOGG::Load(std::string FileName)
	{
		Free();
		Data = new StreamData;

		Data->Alloc.alloc_buffer_length_in_bytes = 256 * 1024;
		Data->Alloc.alloc_buffer = new char[Data->Alloc.alloc_buffer_length_in_bytes];

		Data->Ogg = stb_vorbis_open_filename(FileName.c_str(), NULL, &Data->Alloc);
		stb_vorbis_info info = stb_vorbis_get_info(Data->Ogg);

		this->Size = 0;
		this->Frequency = info.sample_rate;
		this->Channels = info.channels;

		return true;
	}

	void SoundDecoderOGG::Free()
	{
		if (Data != nullptr)
		{
			if (Data->Ogg != nullptr)
			{
				stb_vorbis_close(Data->Ogg);
			}
		}

		Size = 0;
		Frequency = 0;
		Channels = 0;

		delete Data;
	}

	void SoundDecoderOGG::Seek(uint64 Offset)
	{
		stb_vorbis_seek(Data->Ogg, Offset);
	}

	uint32 SoundDecoderOGG::Decode(Sound::Frame* Frames, uint32 Count)
	{
		uint32 TotalSamples = 0;

		while (TotalSamples < Count)
		{
			int Samples = stb_vorbis_get_samples_short_interleaved(Data->Ogg, Channels, (short*)Frames + TotalSamples, (Count - TotalSamples) * 2);

			if (Samples == 0)
			{
				break;
			}

			TotalSamples += Samples;
		}

		return TotalSamples;
	}

	SoundDecoderOGG::~SoundDecoderOGG()
	{
		Free();
	}

}







