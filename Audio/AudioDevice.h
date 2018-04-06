#pragma once

#include <Audio/AudioSource.h>

namespace Columbus
{

	class AudioDevice
	{
	public:
		AudioDevice();

		virtual AudioSource* CreateSource() const;

		virtual ~AudioDevice();
	};

	extern AudioDevice* gAudioDevice;

}










