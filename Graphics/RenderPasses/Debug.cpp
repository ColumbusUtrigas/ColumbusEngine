#include "Core/CVar.h"
#include "Core/Util.h"
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

		// TODO: move to a proper place
		if (ImGui::Begin("Stats"))
		{
			auto rg_cpu = GetProfilerCategoryCPU("RenderGraph");
			auto rg_mem = GetProfileCategoryMemory("RenderGraphMemory");
			auto sc_mem = GetProfileCategoryMemory("SceneMemory");

			if (ImGui::BeginTable("RG CPU", 2))
			{
				ImGui::TableHeader("RG CPU");
				for (auto counter : rg_cpu)
				{
					ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
					ImGui::TableNextColumn(); ImGui::Text("%.2fms", counter->LastTime);
				}
				ImGui::EndTable();
			}

			if (ImGui::BeginTable("RG Memory", 2))
			{
				ImGui::TableHeader("RG Memory");
				for (auto counter : rg_mem)
				{
					double mem;
					const char* postfix = HumanizeBytes(counter->Memory, mem);

					ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
					ImGui::TableNextColumn(); ImGui::Text("%.2f%s", mem, postfix);
				}
				ImGui::EndTable();
			}

			if (ImGui::BeginTable("Scene Memory", 2))
			{
				ImGui::TableHeader("Scene Memory");
				for (auto counter : sc_mem)
				{
					double mem;
					const char* postfix = HumanizeBytes(counter->Memory, mem);

					ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
					ImGui::TableNextColumn(); ImGui::Text("%.2f%s", mem, postfix);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();

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
		}
		ImGui::End();
	}

	bool IsDebugConsoleFocused()
	{
		return DebugConsoleFocus;
	}

	void ShowRenderGraphVisualiser(RenderGraph& Graph)
	{
		RenderGraphDebugInformation Info;
		Graph.GetDebugInformation(Info);

		// TODO: show how resource are used in passes
		// TODO: barriers/transitions
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
			const ImColor tooltip_header_colour(247, 217, 17, 255);

			const ImVec2 window_size = ImGui::GetContentRegionAvail();
			const ImVec2 pos = ImGui::GetCursorScreenPos();
			const ImVec2 rect_size = ImVec2((window_size.x - margin * (num-1)) / num, vertical_size);

			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			ImVec2 current_line_pos = pos;
			ImVec2 current_pos = current_line_pos;
			for (const auto& Pass : Info.Passes)
			{
				const ImVec2 min = current_pos;
				const ImVec2 max = current_pos + rect_size;
				const ImVec2 middle = (min + max) / 2;
				draw_list->AddRectFilled(min, max, pass_colours[(int)Pass.Type]);

				bool hovered = ImGui::GetIO().MousePos.x > min.x && ImGui::GetIO().MousePos.y > min.y && ImGui::GetIO().MousePos.x < max.x && ImGui::GetIO().MousePos.y < max.y;

				if (hovered)
					ImGui::SetTooltip("%s\nId: %i\n", Pass.Name.c_str(), Pass.Id);

				ImVec2 text_size = ImGui::CalcTextSize(Pass.Name.c_str());
				ImVec2 text_pos = middle - text_size / 2;
				draw_list->AddText(text_pos, pass_text_colours[(int)Pass.Type], Pass.Name.c_str());
				current_pos.x += rect_size.x + margin;
			}

			for (const auto& Texture : Info.Textures)
			{
				current_line_pos.y += vertical_size + margin;
				const ImVec2 min = ImVec2(Texture.FirstUsage * rect_size.x, 0) + current_line_pos;
				const ImVec2 max = ImVec2((Texture.LastUsage + 1) * rect_size.x, vertical_size) + current_line_pos;
				const ImVec2 middle = (min + max) / 2;
				draw_list->AddRectFilled(min, max, texture_colour);

				bool hovered = ImGui::GetIO().MousePos.x > min.x && ImGui::GetIO().MousePos.y > min.y && ImGui::GetIO().MousePos.x < max.x && ImGui::GetIO().MousePos.y < max.y;

				if (hovered)
				{
					ImGui::BeginTooltip();
					ImGui::TextColored(tooltip_header_colour, "%s", Texture.DebugName.c_str());
					if (ImGui::BeginTable("tooltip", 2))
					{
						ImGui::TableNextColumn(); ImGui::TextDisabled("Id:");
						ImGui::TableNextColumn(); ImGui::Text("%i", Texture.Id);
						ImGui::TableNextColumn(); ImGui::TextDisabled("Type:");
						ImGui::TableNextColumn(); ImGui::Text("%s", TextureTypeToString(Texture.Desc.Type));
						ImGui::TableNextColumn(); ImGui::TextDisabled("Usage:");
						ImGui::TableNextColumn(); ImGui::Text("%s", TextureUsageToString(Texture.Desc.Usage));
						ImGui::TableNextColumn(); ImGui::TextDisabled("Format:");
						ImGui::TableNextColumn(); ImGui::Text("%s", TextureFormatToString(Texture.Desc.Format));
						ImGui::TableNextColumn(); ImGui::TextDisabled("Versions:");
						ImGui::TableNextColumn(); ImGui::Text("%i", Texture.Version + 1);
						ImGui::TableNextColumn(); ImGui::TextDisabled("Width:");
						ImGui::TableNextColumn(); ImGui::Text("%i", Texture.Desc.Width);
						ImGui::TableNextColumn(); ImGui::TextDisabled("Height:");
						ImGui::TableNextColumn(); ImGui::Text("%i", Texture.Desc.Height);
						ImGui::TableNextColumn(); ImGui::TextDisabled("Depth:");
						ImGui::TableNextColumn(); ImGui::Text("%i", Texture.Desc.Depth);

						double Size;
						const char* SizePostfix = HumanizeBytes(Texture.AllocatedSize, Size);

						ImGui::TableNextColumn(); ImGui::TextDisabled("Size:");
						ImGui::TableNextColumn(); ImGui::Text("%.1f %s", Size, SizePostfix);
						ImGui::EndTable();
					}
					ImGui::EndTooltip();
				}

				ImVec2 text_size = ImGui::CalcTextSize(Texture.DebugName.c_str());
				ImVec2 text_pos = middle - text_size / 2;
				draw_list->AddText(text_pos, texture_text_colour, Texture.DebugName.c_str());
			}
			current_line_pos.y += vertical_size + margin;

			ImGui::SetCursorScreenPos(current_line_pos);
		}
		ImGui::End();
	}
}
