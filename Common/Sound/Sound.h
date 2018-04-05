#pragma once

#include <Core/Types.h>
#include <string>

namespace Columbus
{

	enum SoundFormat
	{
		E_SOUND_FORMAT_WAV,
		E_SOUND_FORMAT_UNKNOWN
	};

	SoundFormat SoundGetFormat(std::string FileName);

	bool SoundIsWAV(std::string FileName); //Check file magic

	uint16* SoundLoadWAV(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	uint16* SoundLoad(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	class Sound
	{
	private:
		uint64 BufferSize; //Size of sound buffer
		uint32 Frequency;  //Sound frequency (22050, 44100)
		uint16 Channels;   //Sound channels (1 for mono, 2 for stereo) 

		uint16* Buffer;
	public:
		Sound();

		bool Load(std::string FileName);
		void Free();

		uint64 GetBufferSize() const;
		uint32 GetFrequency() const;
		uint16 GetChannelsCount() const;
		uint16* GetBuffer() const;
 
		~Sound();
	};

}













