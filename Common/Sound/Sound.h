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

	class SoundDecoder;
	class SoundDecoderOGG;
	class Sound;

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

		bool Load(std::string FileName, bool Stream = false);
		void Free();

		void Seek(uint64 Offset);
		uint32 Decode(Frame* Frames, uint32 Count, uint64 Offset);

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
		uint64 Size;
		uint32 Frequency;
		uint16 Channels;
	public:
		SoundDecoder() {}

		virtual bool Load(std::string FileName) { return false; }
		virtual void Free() {}
		virtual void Seek(uint64 Offset) {}
		virtual uint32 Decode(Sound::Frame* Frames, uint32 Count) { return 0; }

		uint64 GetSize() const { return Size; }
		uint32 GetFrequency() const { return Frequency; }
		uint16 GetChannels() const { return Channels; }

		virtual ~SoundDecoder() {}
	};

	class SoundDecoderOGG : public SoundDecoder
	{
	private:
		struct StreamData;
		StreamData* Data;
	public:
		SoundDecoderOGG();

		bool Load(std::string FileName) override;
		void Free() override;
		void Seek(uint64 Offset) override;
		uint32 Decode(Sound::Frame* Frames, uint32 Count) override;

		~SoundDecoderOGG() override;
	};

}













