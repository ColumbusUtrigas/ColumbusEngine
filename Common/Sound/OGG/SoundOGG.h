#include <Common/Sound/Sound.h>

namespace Columbus
{

	class SoundDecoderOGG : public SoundDecoder
	{
	private:
		struct StreamData;
		StreamData* Data;
	public:
		SoundDecoderOGG();

		static bool IsOGG(const char* FileName);

		bool Load(const char* FileName) override;
		void Free() override;
		void Seek(uint64 Offset) override;
		uint32 Decode(Sound::Frame* Frames, uint32 Count) override;

		~SoundDecoderOGG() override;
	};

}


