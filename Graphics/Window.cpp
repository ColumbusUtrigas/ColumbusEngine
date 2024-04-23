#include "Window.h"

// Third party
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

namespace Columbus
{

	WindowVulkan::WindowVulkan(InstanceVulkan& Instance, SPtr<DeviceVulkan> Device) : Instance(Instance), Device(Device)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		Window = SDL_CreateWindow("Columbus Engine (Vulkan)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Size.X, Size.Y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		SDL_Vulkan_CreateSurface(Window, Instance.instance, &Surface);

		Swapchain = Device->CreateSwapchain(Surface, nullptr);

		for (int i = 0; i < MaxFramesInFlight; i++)
		{
			AcquireImageSemaphores[i] = Device->CreateSemaphore();
			ImageBarrierSemaphores[i] = Device->CreateSemaphore();
			FrameFences[i] = Device->CreateFence(true);
			SwapchainImageBarrierCmdBuffers[i] = Device->CreateCommandBuffer();
		}
	}

	void WindowVulkan::OnResize(const iVector2& NewSize)
	{
		if (Size != NewSize)
		{
			RecreateSwapchain();
		}
	}

	void WindowVulkan::RecreateSwapchain()
	{
		SwapchainVulkan* newSwapchain = Device->CreateSwapchain(Surface, Swapchain);
		delete Swapchain;
		Swapchain = newSwapchain;

		// Size = NewSize;
		Size = iVector2((int)Swapchain->swapChainExtent.width, (int)newSwapchain->swapChainExtent.height);
	}

	WindowSwapchainAcquireData WindowVulkan::AcquireNextSwapchainImage()
	{
		const u64 Index = FrameIndex % MaxFramesInFlight;

		Device->WaitForFence(FrameFences[Index], 18446744073709551615ULL);
		Device->AcqureNextImage(Swapchain, AcquireImageSemaphores[Index], CurrentAcquiredImageId);
		Device->ResetFence(FrameFences[Index]);

		WindowSwapchainAcquireData Result;
		Result.Image = Swapchain->Textures[CurrentAcquiredImageId];
		Result.ImageAcquiredSemaphore = AcquireImageSemaphores[Index];

		return Result;
	}

	void WindowVulkan::Present(VkSemaphore WaitSemaphore, Texture2* ImageToShowInSwapchain)
	{
		assert(CurrentAcquiredImageId != -1 && "Forgot to AcquireNextSwapchainImage first?");

		Texture2* CurrentSwapchainImage = Swapchain->Textures[CurrentAcquiredImageId];

		const u64 Index = FrameIndex % MaxFramesInFlight;
		SwapchainImageBarrierCmdBuffers[Index]->Reset();
		SwapchainImageBarrierCmdBuffers[Index]->Begin();
		SwapchainImageBarrierCmdBuffers[Index]->BeginDebugMarker("Barrier swapchain image");

		if (ImageToShowInSwapchain != nullptr)
		{
			iVector2 ImageSize;
			ImageSize.X = (int)ImageToShowInSwapchain->GetDesc().Width;
			ImageSize.Y = (int)ImageToShowInSwapchain->GetDesc().Height;

			SwapchainImageBarrierCmdBuffers[Index]->TransitionImageLayout(ImageToShowInSwapchain, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
			SwapchainImageBarrierCmdBuffers[Index]->TransitionImageLayout(CurrentSwapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
			SwapchainImageBarrierCmdBuffers[Index]->CopyImage(ImageToShowInSwapchain, CurrentSwapchainImage, {}, {}, iVector3(ImageSize, 1));
		}

		SwapchainImageBarrierCmdBuffers[Index]->TransitionImageLayout(CurrentSwapchainImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		SwapchainImageBarrierCmdBuffers[Index]->EndDebugMarker();
		SwapchainImageBarrierCmdBuffers[Index]->End();
		Device->Submit(SwapchainImageBarrierCmdBuffers[Index], FrameFences[Index], 1, &WaitSemaphore, 1, &ImageBarrierSemaphores[Index]);

		Device->Present(Swapchain, CurrentAcquiredImageId, ImageBarrierSemaphores[Index]);

		FrameIndex++;
	}

	WindowVulkan::~WindowVulkan()
	{
		delete Swapchain;
		vkDestroySurfaceKHR(Instance.instance, Surface, nullptr);
		SDL_DestroyWindow(Window);
	}

}
