#include <Audio/AudioDevice.h>

namespace Columbus
{

	AudioDevice* gAudioDevice = nullptr;

	AudioDevice::AudioDevice() {}

	AudioSource* AudioDevice::CreateSource() const
	{
		return new AudioSource();
	}

	AudioDevice::~AudioDevice() {}

}















