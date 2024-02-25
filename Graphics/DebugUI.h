#pragma once

#include "Core/View.h"
#include "Core/Texture.h"
#include "Window.h"
#include "RenderGraph.h"

// TODO: hide vulkan
#include "Vulkan/DeviceVulkan.h"

// forward declarations
union SDL_Event;

namespace Columbus::DebugUI
{

	struct Context
	{
		WindowVulkan* Window;
		RenderGraph* Graph; // only renders UI

		VkRenderPass InternalRenderPass;
	};

	struct RenderResult
	{
		Texture2* ResultTexture; // valid only before the next frame
		VkSemaphore FinishSemaphore;
	};

	// Core functions

	Context* Create(WindowVulkan* Window);
	void BeginFrame(Context* Ctx);
	void ProcessInputSDL(Context* Ctx, SDL_Event* Event);
	RenderResult Render(Context* Ctx, VkSemaphore WaitSemaphore);
	void EndFrame(Context* Ctx);
	void Destroy(Context* Ctx);

	// Widgets and UI helpers

	// mutates View
	void ShowScreenshotSaveWindow(RenderView& View);

	// TODO: find a way without external force invalidation
	void TextureWidget(Texture2* Texture, Vector2 Size, bool ForceInvalidate = false);

}
