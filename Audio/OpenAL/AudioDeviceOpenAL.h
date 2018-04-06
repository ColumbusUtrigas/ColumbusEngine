#pragma once

#include <Audio/AudioDevice.h>
#include <AL/al.h>
#include <AL/alc.h>

namespace Columbus
{

	class AudioDeviceOpenAL : public AudioDevice
	{
	private:
		ALCdevice* OpenALDevice;
		ALCcontext* OpenALContext;
	public:
		AudioDeviceOpenAL();

		AudioSource* CreateSource() const override;

		~AudioDeviceOpenAL() override;
	};

}











