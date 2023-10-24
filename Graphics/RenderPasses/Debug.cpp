#include "Core/CVar.h"
#include "Graphics/Core/Types.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"

#include "Lib/imgui/backends/imgui_impl_vulkan.h"

namespace Columbus
{
	VkRenderPass ImGuiRenderPass = NULL;
	bool DebugConsoleFocus = false;

	void SetupImguiForSwapchain(SPtr<DeviceVulkan> Device, const SwapchainVulkan* Swapchain)
	{
		static bool Initialised = false;
		if (!Initialised)
		{
			ImGui_ImplVulkan_InitInfo imguiVk{};
			imguiVk.Instance = Device->_Instance;
			imguiVk.PhysicalDevice = Device->_PhysicalDevice;
			imguiVk.Device = Device->_Device;
			imguiVk.QueueFamily = Device->_FamilyIndex;
			imguiVk.Queue = *Device->_ComputeQueue;
			imguiVk.DescriptorPool = Device->_DescriptorPool;
			imguiVk.MinImageCount = Swapchain->minImageCount;
			imguiVk.ImageCount = Swapchain->imageCount;

			AttachmentDesc Attachments[] = {
				AttachmentDesc { "Swapchain", AttachmentType::Color, AttachmentLoadOp::Load, TextureFormat::BGRA8SRGB }
			};

			ImGuiRenderPass = Device->CreateRenderPass(Attachments);

			ImGui_ImplVulkan_Init(&imguiVk, ImGuiRenderPass);
			{
				auto CommandBuffer = Device->CreateCommandBufferShared();
				CommandBuffer->Begin();
				ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->_CmdBuf);
				CommandBuffer->End();
				Device->Submit(CommandBuffer.get());
				Device->QueueWaitIdle();
				ImGui_ImplVulkan_DestroyFontUploadObjects();
			}

			Initialised = true;
		}
	}

	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, RenderGraphTextureRef Texture)
	{
		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, Texture, {} };
		Parameters.ExternalRenderPass = ImGuiRenderPass;
		RenderPassDependencies Dependencies;

		Graph.AddPass("DebugOverlay", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context.CommandBuffer->_CmdBuf);
		});
	}

	void ShowDebugConsole()
	{
		static char buf[1024]{ 0 };
		static std::vector<UPtr<char>> History;

		// TODO: history
		// TODO: autocomplete
		// TODO: show/hide with `
		if (ImGui::Begin("Console"))
		{
			DebugConsoleFocus = ImGui::IsWindowFocused();

			if (ImGui::BeginChild("Scroll", ImVec2(0, -30)))
			{
				DebugConsoleFocus |= ImGui::IsWindowFocused();
				for (const auto& Str : History)
				{
					ImGui::TextWrapped("%s", Str.get());
				}
				ImGui::EndChild();
			}

			ImGui::Separator();
			
			if (DebugConsoleFocus && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere();

			if (ImGui::InputText("Input", buf, 1024, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				char cmd[1024];
				snprintf(cmd, 1024, ">>> %s <<<", buf);

				History.emplace_back(strdup(cmd));
				History.emplace_back(ConsoleVariableSystem::RunConsoleCommand(buf));
				memset(buf, 0, 1024);
			}
			ImGui::End();
		}
	}

	bool IsDebugConsoleFocused()
	{
		return DebugConsoleFocus;
	}

	void ShowRenderGraphVisualiser(RenderGraph& Graph)
	{
		RenderGraphDebugInformation Info;
		Graph.GetDebugInformation(Info);

		// TODO: show all passes on the timeline
		// TODO: show all resources and each one's lifetime between passes
		if (ImGui::Begin("RenderGraph Viz"))
		{
			const float margin = 5;
			const float vertical_size = 50;
			int num = (int)Info.Passes.size();

			const ImColor pass_colours[] = {
				ImColor(232, 92,  16, 255), // RenderGraphPassType::Raster
				ImColor(18, 204, 108, 255), // RenderGraphPassType::Compute
			};

			const ImColor pass_text_colours[] = {
				ImColor(255, 255, 255, 255), // RenderGraphPassType::Raster
				ImColor(18, 18, 18, 255), // RenderGraphPassType::Compute
			};

			const ImColor texture_colour(15, 150, 128, 255);
			const ImColor texture_text_colour(255, 255, 255, 255);

			const ImVec2 window_size = ImGui::GetContentRegionAvail();
			const ImVec2 pos = ImGui::GetCursorScreenPos();
			const ImVec2 rect_size = ImVec2((window_size.x - margin * (num-1)) / num, vertical_size);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			ImVec2 current_line_pos = pos;
			ImVec2 current_pos = current_line_pos;
			for (const auto& Pass : Info.Passes)
			{
				draw_list->AddRectFilled(current_pos, current_pos + rect_size, pass_colours[(int)Pass.Type]);

				ImVec2 middle = (current_pos + current_pos + rect_size) / 2;
				ImVec2 text_size = ImGui::CalcTextSize(Pass.Name.c_str());
				ImVec2 text_pos = middle - text_size / 2;
				draw_list->AddText(text_pos, pass_text_colours[(int)Pass.Type], Pass.Name.c_str());
				current_pos.x += rect_size.x + margin;
			}

			for (const auto& Texture : Info.Textures)
			{
				current_line_pos.y += vertical_size + margin;
				const ImVec2 start_life = ImVec2(Texture.FirstUsage * rect_size.x, 0) + current_line_pos;
				const ImVec2 end_life = ImVec2((Texture.LastUsage + 1) * rect_size.x, vertical_size) + current_line_pos;
				const ImVec2 middle = (start_life + end_life) / 2;
				draw_list->AddRectFilled(start_life, end_life, texture_colour);

				ImVec2 text_size = ImGui::CalcTextSize(Texture.DebugName.c_str());
				ImVec2 text_pos = middle - text_size / 2;
				draw_list->AddText(text_pos, texture_text_colour, Texture.DebugName.c_str());
			}
			current_line_pos.y += vertical_size + margin;

			ImGui::SetCursorScreenPos(current_line_pos);

			ImGui::End();
		}
	}
}
