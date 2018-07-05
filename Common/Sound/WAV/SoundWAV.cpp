#include <Common/Sound/Sound.h>
#include <Common/Sound/WAV/SoundWAV.h>
#include <System/File.h>
#include <Math/MathUtil.h>
#include <Core/Memory.h>
#include <vector>

namespace Columbus
{

	#define FOURCC(str) uint32(((uint8*)(str))[0] | (((uint8*)(str))[1] << 8) | (((uint8*)(str))[2] << 16) | (((uint8*)(str))[3] << 24))
	static int64 GetWAVFormat(std::string FileName, uint64& OutSize, uint16& OutFormat, uint16& OutChannels, uint32& OutFreq, uint16& OutBlock)
	{
		File WAVSoundFile(FileName, "rb");
		if (!WAVSoundFile.IsOpened()) return -1;

		struct FMTChunk
		{
			uint16  Format;
			uint16  Channels;
			uint32  SamplesPerSec;
			uint32  BytesPerSec;
			uint16  Block;
			uint16  SampleBits;
		} FMT;

		int64 BeginOffset = -1;

		uint32 FourCC;
		WAVSoundFile.Read(FourCC);

		if (FourCC == FOURCC("RIFF"))
		{
			WAVSoundFile.SeekCur(8);

			while (!WAVSoundFile.IsEOF())
			{
				uint32 type, size;

				WAVSoundFile.Read(type);
				WAVSoundFile.Read(size);

				if (type == FOURCC("fmt "))
				{
					WAVSoundFile.Read(&FMT, sizeof(FMTChunk), 1);
					WAVSoundFile.SeekCur(size - sizeof(FMTChunk));
				}
				else if (type == FOURCC("data"))
				{
					OutSize = size;
					OutFormat = FMT.Format;
					OutFreq = FMT.SamplesPerSec;
					OutChannels = FMT.Channels;
					OutBlock = FMT.Block;
					BeginOffset = WAVSoundFile.Tell();

					break;
				}
				else
				{
					WAVSoundFile.SeekCur(size);
				}
			}
		}

		return BeginOffset;
	}

	static int16* LoadWAV_PCM(File* WAVSoundFile)
	{
		if (WAVSoundFile != nullptr)
		{
			if (WAVSoundFile->IsOpened())
			{
				uint64 Size = WAVSoundFile->GetSize() - WAVSoundFile->Tell();

				int16* WAVSoundData = new int16[Size / sizeof(int16)];

				if (!WAVSoundFile->ReadBytes(WAVSoundData, Size))
				{
					delete[] WAVSoundData;
					return nullptr;
				}

				return WAVSoundData;
			}
		}

		return nullptr;
	}

	int16* SoundLoadWAV(std::string FileName, uint64& OutSize, uint32& OutFrequency, uint16& OutChannels)
	{
		uint64 Size;
		uint16 Format;
		uint16 Channels;
		uint32 Freq;
		uint16 Block;
		uint64 BeginOffset;

		if ((BeginOffset = GetWAVFormat(FileName, Size, Format, Channels, Freq, Block)) == -1)
		{
			return nullptr;
		}

		File WAVSoundFile(FileName, "rb");
		if (!WAVSoundFile.IsOpened())
		{
			return nullptr;
		}

		WAVSoundFile.SeekSet(BeginOffset);

		switch (Format)
		{
			case 1: /*Microsoft PCM*/
			{
				int16* Data = LoadWAV_PCM(&WAVSoundFile);
				OutSize = Size;
				OutFrequency = Freq;
				OutChannels = Channels;

				return Data;
			}

			case 2: /*Microsoft ADPCM*/
			{
				WAVSoundFile.Close();
				SoundDecoder* Decoder = new SoundDecoderADPCM();
				if (!Decoder->Load(FileName))
				{
					delete Decoder;
					return nullptr;
				}

				std::vector<int16> TmpBuffer;

				uint32 Count = 512;
				Sound::Frame* Frames = new Sound::Frame[512];

				uint64 TotalSamples = 0;

				while (true)
				{
					uint32 Samples = Decoder->Decode(Frames, Count);
					TotalSamples += Samples;

					for (uint32 i= 0; i < Samples; i++)
					{
						TmpBuffer.push_back(Frames[i].L);

						if (Channels == 2)
						{
							TmpBuffer.push_back(Frames[i].R);
						}
					}

					if (Samples < Count)
					{
						break;
					}
				}

				int16* Data = new int16[TmpBuffer.size()];
				std::copy(TmpBuffer.begin(), TmpBuffer.end(), Data);

				OutSize = TotalSamples * sizeof(int16);
				OutFrequency = Freq;
				OutChannels = Channels;

				delete Frames;
				return Data;
			}
		}


		return nullptr;
	}

	SoundDecoderPCM::SoundDecoderPCM() { }

	bool SoundDecoderPCM::IsWAV(std::string FileName)
	{
		uint64 Size;
		uint16 Format;
		uint16 Channels;
		uint32 Freq;
		uint16 Block;

		if (GetWAVFormat(FileName, Size, Format, Channels, Freq, Block) != -1)
		{
			return Format == 1;
		}

		return false;
	}

	bool SoundDecoderPCM::Load(std::string FileName)
	{
		Free();

		WAVSoundFile.Open(FileName, "rb");
		if (!WAVSoundFile.IsOpened())
		{
			return false;
		}

		uint16 Format = 0;
		uint16 Block = 0;

		if ((BeginOffset = GetWAVFormat(FileName, Size, Format, Channels, Frequency, Block)) == -1)
		{
			return false;
		}

		if (Format != 1/*Microsoft PCM*/)
		{
			return false;
		}

		WAVSoundFile.SeekSet(BeginOffset);

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
		if (WAVSoundFile.IsOpened())
		{
			uint32 SampleSize = sizeof(int16) * Channels;

			WAVSoundFile.SeekSet(Offset * SampleSize + BeginOffset);
		}
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
					WAVSoundFile.Read(Frame.L);
					Frame.R = Frame.L;
				}
				else 
				{
					WAVSoundFile.Read(Frame);
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

	SoundDecoderADPCM::SoundDecoderADPCM() { }

	bool SoundDecoderADPCM::IsWAV(std::string FileName)
	{
		uint64 Size;
		uint16 Format;
		uint16 Channels;
		uint32 Freq;
		uint16 Block;

		if (GetWAVFormat(FileName, Size, Format, Channels, Freq, Block) != -1)
		{
			return Format == 2;
		}

		return false;
	}

	bool SoundDecoderADPCM::Load(std::string FileName)
	{
		Free();

		WAVSoundFile.Open(FileName, "rb");
		if (!WAVSoundFile.IsOpened())
		{
			return false;
		}

		uint16 Format = 0;

		if ((BeginOffset = GetWAVFormat(FileName, Size, Format, Channels, Frequency, Block)) == -1)
		{
			return false;
		}

		if (Format != 2/*Microsoft ADPCM*/)
		{
			return false;
		}

		WAVSoundFile.SeekSet(BeginOffset);

		return true;
	}

	void SoundDecoderADPCM::Free()
	{
		WAVSoundFile.Close();
		Size = 0;
		Frequency = 0;
		Channels = 0;
		Block = 0;
	}

	void SoundDecoderADPCM::Seek(uint64 Offset)
	{
		if (WAVSoundFile.IsOpened())
		{
			WAVSoundFile.SeekSet(Offset + BeginOffset);
		}
	}

	int SoundDecoderADPCM::Channel::Predicate(uint8 Nibble)
	{
		static const int Table[] = { 230, 230, 230, 230, 307, 409, 512, 614, 768, 614, 512, 409, 307, 230, 230, 230 };

		int8 Ns = Nibble;
		if (Ns & 8) Ns -= 16;

		int Sample = (Sample1 * C1 + Sample2 * C2) / 256 + Ns * InitialDelta;
		Sample  = Math::Clamp(Sample, -32768, 32767);
		Sample2 = Sample1;
		Sample1 = Sample;
		InitialDelta   = Math::Max(Table[Nibble] * InitialDelta / 256, 16);

		return Sample;
	}

	uint32 SoundDecoderADPCM::Decode(Sound::Frame* Frames, uint32 Count)
	{
		static const int AdaptCoeff1[] = { 256, 512, 0, 192, 240, 460, 392 };
		static const int AdaptCoeff2[] = { 0, -256, 0, 64, 0, -208, -232 };

		uint32 TotalSamples = 0;

		while (TotalSamples < Count)
		{
			if (WAVSoundFile.IsEOF())
			{
				break;
			}

			if ((WAVSoundFile.Tell() - BeginOffset) % Block == 0)
			{
				for (uint32 i = 0; i < Channels; i++)
				{
					uint8 Index;
					WAVSoundFile.Read(Index);

					Chans[i].C1 = AdaptCoeff1[Index];
					Chans[i].C2 = AdaptCoeff2[Index];
				}

				for (uint32 i = 0; i < Channels; i++) WAVSoundFile.Read(Chans[i].InitialDelta);
				for (uint32 i = 0; i < Channels; i++) WAVSoundFile.Read(Chans[i].Sample1);
				for (uint32 i = 0; i < Channels; i++) WAVSoundFile.Read(Chans[i].Sample2);

				if (Channels == 1)
				{
					Frames[TotalSamples].L = Frames[TotalSamples].R = Chans[0].Sample2; TotalSamples++;
					Frames[TotalSamples].L = Frames[TotalSamples].R = Chans[0].Sample1; TotalSamples++;
				}
				else
				{
					Frames[TotalSamples].L = Chans[0].Sample2;
					Frames[TotalSamples].R = Chans[1].Sample2; TotalSamples++;
					Frames[TotalSamples].L = Chans[0].Sample1;
					Frames[TotalSamples].R = Chans[1].Sample1; TotalSamples++;
				}
			}
			else
			{
				uint8 Value;
				WAVSoundFile.Read(Value);

				uint8 N1 = Value >> 4;
				uint8 N2 = Value & 0xF;

				if (Channels == 1)
				{
					Frames[TotalSamples].L = Frames[TotalSamples].R = Chans[0].Predicate(N1); TotalSamples++;
					Frames[TotalSamples].L = Frames[TotalSamples].R = Chans[0].Predicate(N2); TotalSamples++;
				}
				else
				{
					Frames[TotalSamples].L = Chans[0].Predicate(N1);
					Frames[TotalSamples].R = Chans[1].Predicate(N2);
					TotalSamples++;
				}
			}
		}

		return TotalSamples;
	}

	SoundDecoderADPCM::~SoundDecoderADPCM()
	{
		Free();
	}

	#undef FOURCC

}











