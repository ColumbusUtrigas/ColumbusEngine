#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

#include "Lib/imgui/backends/imgui_impl_vulkan.h"

namespace Columbus
{
	void InitImguiRendering(SPtr<DeviceVulkan> Device, SwapchainVulkan* Swapchain)
	{
		// that's a hacky way to work around a need to have a renderpass before render graph is built
		// we manually initialise internal backend struct so fonts can be built

		ImGui_ImplVulkan_InitInfo imguiVk{};
		imguiVk.Instance = Device->_Instance;
		imguiVk.PhysicalDevice = Device->_PhysicalDevice;
		imguiVk.Device = Device->_Device;
		imguiVk.QueueFamily = Device->_FamilyIndex;
		imguiVk.Queue = *Device->_ComputeQueue;
		imguiVk.DescriptorPool = Device->_DescriptorPool;
		imguiVk.MinImageCount = Swapchain->minImageCount;
		imguiVk.ImageCount = Swapchain->imageCount;

		ImGui::GetIO().BackendRendererUserData = (void*)&imguiVk;

		auto CommandBuffer = Device->CreateCommandBufferShared();
		CommandBuffer->Reset();
		CommandBuffer->Begin();
		ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->_CmdBuf);

		VkSubmitInfo end_info = {};
		end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		end_info.commandBufferCount = 1;
		end_info.pCommandBuffers = &CommandBuffer->_CmdBuf;
		vkEndCommandBuffer(CommandBuffer->_CmdBuf);
		vkQueueSubmit(*Device->_ComputeQueue, 1, &end_info, VK_NULL_HANDLE);

		vkDeviceWaitIdle(Device->_Device);
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		ImGui::GetIO().BackendRendererUserData = nullptr;
	}

	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Texture, {} };
		RenderPassDependencies Dependencies;

		Graph.AddPass("DebugOverlay", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			// TODO: use RHI instead of imgui vulkan
			static bool Initialised = false;
			static ImGui_ImplVulkan_InitInfo imguiVk{};

			if (!Initialised)
			{
				imguiVk.Instance = Context.Device->_Instance;
				imguiVk.PhysicalDevice = Context.Device->_PhysicalDevice;
				imguiVk.Device = Context.Device->_Device;
				imguiVk.QueueFamily = Context.Device->_FamilyIndex;
				imguiVk.Queue = *Context.Device->_ComputeQueue;
				imguiVk.DescriptorPool = Context.Device->_DescriptorPool;
				imguiVk.MinImageCount = View.Swapchain->minImageCount;
				imguiVk.ImageCount = View.Swapchain->imageCount;

				ImGui_ImplVulkan_Init(&imguiVk, Context.VulkanRenderPass);
				{
					auto CommandBuffer = Context.Device->CreateCommandBufferShared();
					CommandBuffer->Begin();
					ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->_CmdBuf);
					CommandBuffer->End();
					Context.Device->Submit(CommandBuffer.get());
					Context.Device->QueueWaitIdle();
					ImGui_ImplVulkan_DestroyFontUploadObjects();
				}
				Initialised = true;

				ImGui::NewFrame();
			}

			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context.CommandBuffer->_CmdBuf);
		});
	}
}
