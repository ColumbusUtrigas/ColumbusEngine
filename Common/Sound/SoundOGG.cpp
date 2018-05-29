#include <Common/Sound/Sound.h>
#include <System/File.h>
#include <stb_vorbis.h>

namespace Columbus
{

	bool SoundIsOGG(std::string FileName)
	{
		File OGGSoundFile(FileName, "rb");
		if (!OGGSoundFile.IsOpened()) return false;

		uint8 Magic[4];
		OGGSoundFile.Read(Magic, sizeof(Magic), 1);
		OGGSoundFile.Close();

		if (Magic[0] == 'O' &&
		    Magic[1] == 'g' &&
		    Magic[2] == 'g' &&
		    Magic[3] == 'S')
		{
			return true;
		}

		return false;
	}

	int16* SoundLoadOGG(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		int16* Ret;
		OutSize = stb_vorbis_decode_filename(FileName.c_str(), (int*)&OutChannels, (int*)&OutFrequency, &Ret) * OutChannels * sizeof(uint16);
		return Ret;
	}

}







