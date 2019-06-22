#include <Common/Sound/Sound.h>
#include <System/File.h>
#define MINIMP3_IMPLEMENTATION
#include <minimp3.h>
#include <vector>

namespace Columbus
{

	bool SoundIsMP3(const char* FileName)
	{
		File MP3SoundFile(FileName, "rb");
		if (!MP3SoundFile.IsOpened()) return false;

		uint8 Magic[3];
		MP3SoundFile.Read(Magic, sizeof(Magic), 1);
		MP3SoundFile.Close();

		if ((Magic[0] == 0xFF &&
		     Magic[1] == 0xFB) ||
			(Magic[0] == 'I' &&
		     Magic[1] == 'D' &&
		     Magic[2] == '3'))
		{
			return true;
		}

		return false;
	}

	int16* SoundLoadMP3(const char* FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		File MP3SoundFile(FileName, "rb");
		if (!MP3SoundFile.IsOpened()) return nullptr;

		uint64 MP3Size = MP3SoundFile.GetSize();

		uint8* Buffer = new uint8[MP3SoundFile.GetSize()];
		MP3SoundFile.Read(Buffer, MP3Size, 1);

		MP3SoundFile.Close();

		OutSize = 0;
		OutFrequency = 0;
		OutChannels = 0;

		int16* Ret;
		int16* PCM = new int16[MINIMP3_MAX_SAMPLES_PER_FRAME];
		std::vector<int16> Data;

		mp3dec_t MP3Decoder;
		mp3dec_frame_info_t MP3FrameInfo;

		mp3dec_init(&MP3Decoder);
		memset(&MP3FrameInfo, 0, sizeof(mp3dec_frame_info_t));

		uint64 TotalSamples = 0;
		uint64 Offset = 0;

		do
		{
			uint32 Samples = mp3dec_decode_frame(&MP3Decoder, Buffer, MP3Size, PCM, &MP3FrameInfo);
			TotalSamples += Samples;

			if (Samples != 0)
			{
				for (uint32 i = 0; i < Samples * MP3FrameInfo.channels; i++)
				{
					Data.push_back(PCM[i]);
				}
			}

			Offset += MP3FrameInfo.frame_bytes;
			Buffer += MP3FrameInfo.frame_bytes;
			MP3Size -= MP3FrameInfo.frame_bytes;
		} while (MP3FrameInfo.frame_bytes);

		OutSize = Data.size() * sizeof(int16);
		OutFrequency = MP3FrameInfo.hz;
		OutChannels = MP3FrameInfo.channels;

		Ret = new int16[TotalSamples * MP3FrameInfo.channels];
		std::copy(Data.begin(), Data.end(), Ret);

		Buffer -= Offset;
		delete[] Buffer;
		delete[] PCM;
		return Ret;
	}

}




















