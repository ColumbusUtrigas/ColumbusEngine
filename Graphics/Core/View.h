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
		Camera CameraCur;
		Camera CameraPrev;
		DebugRender* DebugRender = nullptr;
	};

}
