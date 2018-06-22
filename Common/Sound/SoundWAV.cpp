#include <Common/Sound/Sound.h>
#include <System/File.h>

namespace Columbus
{

	typedef struct 
	{
		uint8 RIFFMagic[4]; //Magic bytes "RIFF"
		uint32 Size;        //Size of whole file (In WAVE file it Size - 8)
		uint8 WAVEMagic[4]; //Magic bytes "WAVE"
		uint8 FMTMagic[4];  //Magic bytes "fmt "
		uint32 SChunkSize;  //Must be 16
		uint16 Format;      //1 if PCM
		uint16 Channels;    //1 for mono, 2 for stereo
		uint32 Frequency;   //Sound fequency (22050Hz, 44100Hz, etc...)
		uint32 ByteRate;    //Bytes per second
		uint16 BlockAlign;  //Bytes per sample including all channels
		uint16 BPS;         //Bits per sample (8, 16, etc...)
		uint8 DataMagic[4]; //Magic bytes "data"
		uint32 DataSize;    //Size of sound data
	} WAV_HEADER;

	static bool ReadHeader(WAV_HEADER* Header, File* InFile)
	{
		if (Header == nullptr || InFile == nullptr) return false;

		if (!InFile->ReadBytes(Header->RIFFMagic, sizeof(Header->RIFFMagic))) return false;
		if (!InFile->ReadUint32(&Header->Size)) return false;
		if (!InFile->ReadBytes(Header->WAVEMagic, sizeof(Header->WAVEMagic))) return false;
		if (!InFile->ReadBytes(Header->FMTMagic, sizeof(Header->FMTMagic))) return false;
		if (!InFile->ReadUint32(&Header->SChunkSize)) return false;
		if (!InFile->ReadUint16(&Header->Format)) return false;
		if (!InFile->ReadUint16(&Header->Channels)) return false;
		if (!InFile->ReadUint32(&Header->Frequency)) return false;
		if (!InFile->ReadUint32(&Header->ByteRate)) return false;
		if (!InFile->ReadUint16(&Header->BlockAlign)) return false;
		if (!InFile->ReadUint16(&Header->BPS)) return false;
		if (!InFile->ReadBytes(Header->DataMagic, sizeof(Header->DataMagic))) return false;
		if (!InFile->ReadUint32(&Header->DataSize)) return false;

		return true;
	}

	bool SoundIsWAV(std::string FileName)
	{
		File WAVSoundFile(FileName, "rb");
		if (!WAVSoundFile.IsOpened()) return false;

		uint8 Magic[12];
		WAVSoundFile.Read(Magic, sizeof(Magic), 1);
		WAVSoundFile.Close();

		if (Magic[0] == 'R' &&
		    Magic[1] == 'I' &&
		    Magic[2] == 'F' &&
		    Magic[3] == 'F')
		{
			//This file format is RIFF
			if (Magic[8] == 'W' &&
			    Magic[9] == 'A' &&
			    Magic[10] == 'V' &&
			    Magic[11] == 'E')
			{
				//This file format is WAVE
				return true;
			}
		}

		return false;
	}

	int16* SoundLoadWAV(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		File WAVSoundFile(FileName, "rb");
		if (!WAVSoundFile.IsOpened())
		{
			return nullptr;
		}

		WAV_HEADER Header;

		if (!ReadHeader(&Header, &WAVSoundFile))
		{
			return nullptr;
		}

		int16* WAVSoundData = new int16[Header.DataSize / sizeof(int16)];

		if (!WAVSoundFile.ReadBytes(WAVSoundData, Header.DataSize))
		{
			delete[] WAVSoundData;
			return nullptr;
		}

		OutSize = Header.DataSize;
		OutFrequency = Header.Frequency;
		OutChannels = Header.Channels;

		return WAVSoundData;
	}

	SoundDecoderPCM::SoundDecoderPCM() { }

	bool SoundDecoderPCM::Load(std::string FileName)
	{
		Free();
		if (!SoundIsWAV(FileName))
		{
			return false;
		}

		WAVSoundFile.Open(FileName, "rb");

		if (!WAVSoundFile.IsOpened())
		{
			return false;
		}

		WAV_HEADER Header;

		if (!ReadHeader(&Header, &WAVSoundFile))
		{
			WAVSoundFile.Close();
			return false;
		}

		Size = WAVSoundFile.GetSize() - sizeof(WAV_HEADER);
		Frequency = Header.Frequency;
		Channels = Header.Channels;

		return true;
	}

	void SoundDecoderPCM::Free()
	{
		WAVSoundFile.Close();
		Size = 0;
		Frequency = 0;
		Channels = 0;
	}

	void SoundDecoderPCM::Seek(uint64 Offset)
	{

	}

	uint32 SoundDecoderPCM::Decode(Sound::Frame* Frames, uint32 Count)
	{
		if (Frames != nullptr && Count != 0 && WAVSoundFile.IsOpened())
		{
			Sound::Frame Frame;
			uint32 Samples = 0;

			for (uint32 i = 0; i < Count; i++)
			{
				if (WAVSoundFile.IsEOF())
				{
					return Samples;
				}

				if (Channels == 1)
				{
					WAVSoundFile.Read(&Frame.L, sizeof(int16), 1);
					Frame.R = Frame.L;
				}
				else 
				{
					WAVSoundFile.Read(&Frame, sizeof(Sound::Frame), 1);
				}

				Frames[i] = Frame;
				Samples++;
			}

			return Samples;
		}

		return 0;
	}

	SoundDecoderPCM::~SoundDecoderPCM()
	{
		Free();
	}

}














