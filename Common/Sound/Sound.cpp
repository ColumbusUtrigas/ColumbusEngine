#include <Common/Sound/Sound.h>
#include <Core/Memory.h>

namespace Columbus
{

	SoundFormat SoundGetFormat(std::string FileName)
	{
		if (SoundIsWAV(FileName)) return SoundFormat::WAV;
		if (SoundIsOGG(FileName)) return SoundFormat::OGG;

		return SoundFormat::Unknown;
	}

	int16* SoundLoad(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		SoundFormat Format = SoundGetFormat(FileName);

		switch (Format)
		{
		case SoundFormat::Unknown: return nullptr; break;
		case SoundFormat::WAV: return SoundLoadWAV(FileName, OutSize, OutFrequency, OutChannels); break;
		case SoundFormat::OGG: return SoundLoadOGG(FileName, OutSize, OutFrequency, OutChannels); break;
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
		Buffer(nullptr)
	{ }
	
	bool Sound::Load(std::string FileName)
	{
		Buffer = SoundLoad(FileName, BufferSize, Frequency, Channels);

		return Buffer != nullptr;
	}

	void Sound::Free()
	{
		BufferSize = 0;
		Frequency = 0;
		Channels = 0;
		//delete[] Buffer;
		if (Buffer != nullptr) Memory::Free(Buffer);
	}

	uint64 Sound::GetBufferSize() const
	{
		return BufferSize;
	}

	uint32 Sound::GetFrequency() const
	{
		return Frequency;
	}

	uint16 Sound::GetChannelsCount() const
	{
		return Channels;
	}

	int16* Sound::GetBuffer() const
	{
		return Buffer;
	}

	Sound::~Sound()
	{
		Free();
	}

}









