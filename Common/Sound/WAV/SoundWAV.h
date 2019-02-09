#include <Common/Sound/Sound.h>
#include <System/File.h>

namespace Columbus
{

	class SoundDecoderPCM : public SoundDecoder
	{
	private:
		File WAVSoundFile;
	public:
		SoundDecoderPCM();

		static bool IsWAV(const char* FileName);

		bool Load(const char* FileName) override;
		void Free() override;
		void Seek(uint64 Offset) override;
		uint32 Decode(Sound::Frame* Frames, uint32 Count) override;

		~SoundDecoderPCM() override;
	};

	/*
	* @see: https://wiki.multimedia.cx/?title=Microsoft_ADPCM
	*/
	class SoundDecoderADPCM : public SoundDecoder
	{
	private:
		File WAVSoundFile;
		uint16 Block;

		struct Channel
		{
			int16 C1, C2;
			int16 InitialDelta;
			int16 Sample1;
			int16 Sample2;

			int Predicate(uint8 Nibble);
		} Chans[2];
	public:
		SoundDecoderADPCM();

		static bool IsWAV(const char* FileName);

		bool Load(const char* FileName) override;
		void Free() override;
		void Seek(uint64 Offset) override;
		uint32 Decode(Sound::Frame* Frames, uint32 Count) override;

		~SoundDecoderADPCM() override;
	};

}




















