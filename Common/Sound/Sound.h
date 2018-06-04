#pragma once

#include <Core/Types.h>
#include <string>

namespace Columbus
{

	enum class SoundFormat
	{
		WAV,
		OGG,
		Unknown
	};

	SoundFormat SoundGetFormat(std::string FileName);

	bool SoundIsWAV(std::string FileName); //Check file magic
	bool SoundIsOGG(std::string FileName); //Check file magic

	int16* SoundLoadWAV(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);
	int16* SoundLoadOGG(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	int16* SoundLoad(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	class Sound
	{
	private:
		uint64 BufferSize; //Size of sound buffer
		uint32 Frequency;  //Sound frequency (22050, 44100)
		uint16 Channels;   //Sound channels (1 for mono, 2 for stereo) 

		int16* Buffer;
	public:
		struct Frame
		{
			int16 L;
			int16 R;
		};
	public:
		Sound();

		bool Load(std::string FileName);
		void Free();

		uint64 GetBufferSize() const;
		uint32 GetFrequency() const;
		uint16 GetChannelsCount() const;
		int16* GetBuffer() const;
 
		~Sound();
	};

}













