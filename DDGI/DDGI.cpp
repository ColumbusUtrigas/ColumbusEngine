#include "Defines.h"
#include <rtxgi/ddgi/DDGIVolume.h>

// for SDK implementation
#define _countof(x) (sizeof(x) / sizeof(x[0]))

#include "SDK/src/Math.cpp"
#include "SDK/src/ddgi/DDGIVolume.cpp"

#include "SDK/include/rtxgi/VulkanExtensions.h"
#include "SDK/src/VulkanExtensions.cpp"
#include "SDK/src/ddgi/gfx/DDGIVolume_VK.cpp"

#include "DDGI.h"

namespace Columbus::DDGI
{

void CreateDDGIVolume(DDGIVolume& Volume, SPtr<DeviceVulkan> Device, CommandBufferVulkan& CmdBuf)
{
	COLUMBUS_ASSERT_MESSAGE(Volume.pInternal == nullptr, "DDGIVolume was already initialised");

	// cmdbuf
	rtxgi::DDGIVolumeDesc Desc;
	Desc.probeCounts = { 8, 8, 8 };
	Desc.probeSpacing = { 100, 100, 100 };

	rtxgi::vulkan::DDGIVolumeResources Resources;

	// rtxgi::DDGIRootConstants constants;
	rtxgi::vulkan::DDGIVolume* sdkVolume = new rtxgi::vulkan::DDGIVolume();
	rtxgi::ERTXGIStatus Status = sdkVolume->Create(CmdBuf._CmdBuf, Desc, Resources);
	Volume.pInternal = sdkVolume;

	COLUMBUS_ASSERT(Status == rtxgi::ERTXGIStatus::OK);
}

}