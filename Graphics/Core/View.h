#pragma once

#include "Math/Vector2.h"
#include "Math/Matrix.h"
#include "Graphics/Camera.h"
#include "Graphics/Core/DebugRender.h"
#include "Graphics/Vulkan/SwapchainVulkan.h"

namespace Columbus
{

	struct RenderView
	{
		SwapchainVulkan* Swapchain;
		iVector2 OutputSize;
		iVector2 RenderSize; // internal resolution
		Camera CameraCur;
		Camera CameraPrev;
		DebugRender* DebugRender = nullptr;

		bool ScreenshotHDR = false; // if true, screenshot is done before tonemapping
		char* ScreenshotPath = nullptr; // when is not nullptr, saves a screenshot on disk
	};

}
