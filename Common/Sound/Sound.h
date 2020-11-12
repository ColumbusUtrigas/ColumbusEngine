#pragma once

#include <Core/Types.h>

namespace Columbus
{

	enum class SoundFormat
	{
		WAV_PCM,
		WAV_ADPCM,
		OGG,
		MP3,
		Unknown
	};

	class SoundDecoder;
	class Sound;

	SoundFormat SoundGetFormat(const char* FileName);
	
	bool SoundIsMP3(const char* FileName);

	int16* SoundLoadWAV(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);
	int16* SoundLoadOGG(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);
	int16* SoundLoadMP3(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	int16* SoundLoad(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels);

	class Sound
	{
	private:
		uint64 BufferSize; //Size of sound buffer
		uint32 Frequency;  //Sound frequency (22050, 44100)
		uint16 Channels;   //Sound channels (1 for mono, 2 for stereo) 

		int16* Buffer;

		bool Streaming;
		SoundDecoder* Decoder;
	public:
		struct Frame
		{
			int16 L;
			int16 R;
		};

		struct FrameHight
		{
			int32 L;
			int32 R;
		};
	public:
		Sound();

		bool Load(const char* FileName, bool Stream = false);
		void Free();

		void Seek(uint64 Offset);
		uint32 Decode(Frame* Frames, uint32 Count, uint64& Offset);

		uint64 GetBufferSize() const;
		uint32 GetFrequency() const;
		uint16 GetChannelsCount() const;
		int16* GetBuffer() const;
		double GetLength() const;
		bool IsStreaming() const;
 
		~Sound();
	};

	class SoundDecoder
	{
	protected:
		uint64 Size = 0;
		 int64 BeginOffset = 0;
		uint32 Frequency = 0;
		uint16 Channels = 0;
	public:
		SoundDecoder() {}

		virtual bool Load(const char* FileName) = 0;
		virtual void Free() = 0;
		virtual void Seek(uint64 Offset) = 0;
		virtual uint32 Decode(Sound::Frame* Frames, uint32 Count) = 0;

		uint64 GetSize() const { return Size; }
		uint32 GetFrequency() const { return Frequency; }
		uint16 GetChannels() const { return Channels; }

		virtual ~SoundDecoder() {}
	};

	

}


