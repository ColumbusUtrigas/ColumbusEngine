#pragma once

#include <Math/Vector2.h>

// TODO: hide vulkan
#include "Vulkan/InstanceVulkan.h"
#include "Vulkan/DeviceVulkan.h"

// forward declarations
struct SDL_Window;

namespace Columbus
{

	struct WindowSwapchainAcquireData
	{
		Texture2* Image;
		VkSemaphore ImageAcquiredSemaphore; // is signaled when next image is ready
	};

	struct WindowVulkan
	{
		WindowVulkan(InstanceVulkan& Instance, SPtr<DeviceVulkan> Device);

		void SetVSync(bool VSync);
		void OnResize(const iVector2& NewSize);
		void RecreateSwapchain();

		iVector2 GetSize() const { return Size; }

		// begin frame with this
		WindowSwapchainAcquireData AcquireNextSwapchainImage();

		// end frame with this
		void Present(VkSemaphore WaitSemaphore, Texture2* ImageToShowInSwapchain = nullptr);

		~WindowVulkan();

	public:
		SPtr<DeviceVulkan> Device;
		InstanceVulkan& Instance;

		SDL_Window* Window;
		VkSurfaceKHR Surface;
		SwapchainVulkan* Swapchain;

		VkSemaphore AcquireImageSemaphores[MaxFramesInFlight];
		VkSemaphore ImageBarrierSemaphores[MaxFramesInFlight];

		SPtr<FenceVulkan> FrameFences[MaxFramesInFlight];
		CommandBufferVulkan* SwapchainImageBarrierCmdBuffers[MaxFramesInFlight];

		u32 CurrentAcquiredImageId = -1;
		u64 FrameIndex = 0; // always increments

		iVector2 Size{ 1280, 720 };
	};

}












