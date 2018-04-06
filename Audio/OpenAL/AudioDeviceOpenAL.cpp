#include <Audio/OpenAL/AudioDeviceOpenAL.h>
#include <Audio/OpenAL/AudioSourceOpenAL.h>

namespace Columbus
{

	AudioDeviceOpenAL::AudioDeviceOpenAL()
	{
		OpenALDevice = alcOpenDevice(NULL);
		OpenALContext = alcCreateContext(OpenALDevice, NULL);
		alcMakeContextCurrent(OpenALContext);
	}

	AudioSource* AudioDeviceOpenAL::CreateSource() const
	{
		return new AudioSourceOpenAL();
	}

	AudioDeviceOpenAL::~AudioDeviceOpenAL()
	{
		alcDestroyContext(OpenALContext);
		alcCloseDevice(OpenALDevice); 
	}

}









