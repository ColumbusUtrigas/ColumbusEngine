#pragma once

// this module is just a wrapper around RTXGI SDK to help to hook it into the renderer
// and stop SDK headers from spreading around

#include "../Graphics/Vulkan/DeviceVulkan.h"

struct DDGIVolume
{
	void* pInternal = nullptr; // will be initialised internally as rtxgi::ddgi::DDGIVolume
};

namespace Columbus::DDGI
{
	// TODO: pass all needed shaders
	void CreateDDGIVolume(DDGIVolume& Volume, SPtr<DeviceVulkan> Device, CommandBufferVulkan& CmdBuf);
	// void DestroyDDGIVolume();
}