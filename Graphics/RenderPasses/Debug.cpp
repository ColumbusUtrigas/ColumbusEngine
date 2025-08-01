#include "Core/CVar.h"
#include "Core/Util.h"
#include "Core/Asset.h"
#include "Graphics/Core/Types.h"
#include "Graphics/IrradianceVolume.h"
#include "Graphics/RenderGraph.h"
#include "RenderPasses.h"
#include <algorithm>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

#include "Lib/imgui/backends/imgui_impl_vulkan.h"

ConsoleVariable<bool> CVar_DebugOverlay("r.DebugOverlay", "Enable using debug overlay", true);
ConsoleVariable<bool> CVar_DebugOverlayIrradiance("r.DebugOverlay.Irradiance", "Enable irradiance volume visualisation", true);

namespace Columbus
{
	bool DebugConsoleFocus = false;

	struct DebugObjectParameters
	{
		Matrix Model;
		Matrix VP;
		Vector4 Colour;
		Vector4 Vertices[3];
		u32 Type;
	};

	struct DebugIrradianceProbesParameters
	{
		Matrix View, Projection;

		Vector4 Position;
		Vector4 Extent;
		iVector4 ProbesCount;
		iVector4 ProbeIndex;
		Vector4 TestPoint;
	};

	void DebugOverlayPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, RenderGraphTextureRef OverlayTexture)
	{
		if (!CVar_DebugOverlay.GetValue())
		{
			return;
		}

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, OverlayTexture, {} };
		Parameters.DepthStencilAttachment = RenderPassAttachment{ AttachmentLoadOp::Load, Textures.GBufferDS, AttachmentClearValue{ {}, 1.0f, 0 } };
		Parameters.ViewportSize = Graph.GetTextureSize2D(OverlayTexture);

		RenderPassDependencies Dependencies(Graph.Allocator);

		Graph.AddPass("DebugOverlay", RenderGraphPassType::Raster, Parameters, Dependencies, [View](RenderGraphContext& Context)
		{
			// TODO: normal shader system please
			static GraphicsPipeline* Pipeline = nullptr;
			static GraphicsPipeline* IrradianceVolumePipeline = nullptr;
			if (Pipeline == nullptr)
			{
				{
					GraphicsPipelineDesc Desc;
					Desc.Name = "Debug";
					Desc.rasterizerState.Cull = CullMode::No;
					Desc.blendState.RenderTargets = {
						RenderTargetBlendDesc {
							.BlendEnable = true,
							.SrcBlend = Blend::SrcAlpha,
							.DestBlend = Blend::InvSrcAlpha,
						},
					};
					// Desc.rasterizerState.Fill = FillMode::Wireframe;

					Desc.depthStencilState.DepthEnable = true; // TODO: use flag, so generate two permutations
					Desc.depthStencilState.DepthWriteMask = false;
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Debug.csd");

					Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
				}

				{
					GraphicsPipelineDesc Desc;
					Desc.Name = "IrradianceProbesVisualise";
					Desc.rasterizerState.Cull = CullMode::No;
					Desc.blendState.RenderTargets = {
						RenderTargetBlendDesc {},
					};

					Desc.depthStencilState.DepthEnable = true;
					Desc.depthStencilState.DepthWriteMask = true;

					// TODO:
					Log::Warning("TODO: Skipping Irradiance Volume Visualisation shader - TBD");

#if 0
					Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/IrradianceVolume/IrradianceVolumeVisualise.csd");
					IrradianceVolumePipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
#endif
				}
			}

			for (const DebugRenderObject& Object : View.DebugRender.Objects)
			{
				DebugObjectParameters Parameters {
					.Model = Object.Transform,
					.VP = View.CameraCur.GetViewProjection(),
					.Colour = Object.Colour,
					.Type = (u32)Object.Type,
				};

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);

				if (Object.Type == DebugRenderObjectType::Box)
				{
					Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
					Context.CommandBuffer->Draw(36, 1, 0, 0);
				}
				else if (Object.Type == DebugRenderObjectType::Tri)
				{
					Parameters.Vertices[0] = Vector4(Object.Vertices[0], 1);
					Parameters.Vertices[1] = Vector4(Object.Vertices[1], 1);
					Parameters.Vertices[2] = Vector4(Object.Vertices[2], 1);

					Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
					Context.CommandBuffer->Draw(3, 1, 0, 0);
				}
			}

			if (CVar_DebugOverlayIrradiance.GetValue())
			{
				for (IrradianceVolume& Volume : Context.Scene->IrradianceVolumes)
				{
					if (Volume.ProbesBuffer == nullptr)
						continue;

					Context.CommandBuffer->BindGraphicsPipeline(IrradianceVolumePipeline);

					DebugIrradianceProbesParameters Parameters {
						.View = View.CameraCur.GetViewMatrix(),
						.Projection = View.CameraCur.GetProjectionMatrix(),
						.Position = Vector4(Volume.Position, 0),
						.Extent = Vector4(Volume.Extent, 0),
						.ProbesCount = iVector4(Volume.ProbesCount, 0),
						.ProbeIndex = iVector4(0, 0, 0, 0),
						.TestPoint = Vector4(Volume.TestPoint, 0),
					};

					auto Set = Context.GetDescriptorSet(IrradianceVolumePipeline, 0);
					Context.Device->UpdateDescriptorSet(Set, 0, 0, Volume.ProbesBuffer);

					Context.CommandBuffer->BindDescriptorSetsGraphics(IrradianceVolumePipeline, 0, 1, &Set);
					Context.CommandBuffer->PushConstantsGraphics(IrradianceVolumePipeline, ShaderType::Vertex | ShaderType::Pixel, 0, sizeof(Parameters), &Parameters);
					Context.CommandBuffer->Draw(6 * (Volume.GetTotalProbes()+4), 1, 0, 0);

					for (int i = 0; i < Volume.ProbesCount.X; i++)
					{
						for (int j = 0; j < Volume.ProbesCount.Y; j++)
						{
							for (int k = 0; k < Volume.ProbesCount.Z; k++)
							{
								
							}
						}
					}
				}
			}
		});
	}

	RenderGraphTextureRef DebugVisualisationPass(RenderGraph& Graph, const RenderView& View, SceneTextures& Textures, DeferredRenderContext& DeferredContext)
	{
		iVector2 Size = View.RenderSize;

		TextureDesc2 ResultDesc;
		ResultDesc.Format = TextureFormat::R11G11B10F; // format to get less banding
		ResultDesc.Usage = TextureUsage::RenderTargetColor;
		ResultDesc.Width = (u32)Size.X;
		ResultDesc.Height = (u32)Size.Y;

		RenderGraphTextureRef Result = Graph.CreateTexture(ResultDesc, "VisualisationResult");

		RenderPassParameters Parameters;
		Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, Result, {} };
		Parameters.ViewportSize = Size;

		RenderPassDependencies Dependencies(Graph.Allocator);

		struct ShaderParameters
		{
			enum
			{
				FLAG_NONE   = 0,
				FLAG_R_ONLY = 1,
				FLAG_G_ONLY = 2,
				FLAG_NORMAL = 4,
				FLAG_DEPTH  = 8,
			};

			iVector2 ViewportSize;
			u32 Flags;
		};

		struct VisualisationImageParameters
		{
			RenderGraphTextureRef TextureToVisualise;
			TextureBindingFlags BindingFlags;
			ShaderParameters ShaderParams;
		};

		VisualisationImageParameters ImagesParams[6]{};
		int ImagesParamsCount = 1;

		ImagesParams[0].BindingFlags = TextureBindingFlags::AspectColour;
		ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_NONE;

		switch (DeferredContext.VisualisationMode)
		{
		case EDeferredRenderVisualisationMode::GBufferOverview:
			ImagesParamsCount = 6;

			ImagesParams[0].TextureToVisualise = Textures.GBufferAlbedo;
			ImagesParams[1].TextureToVisualise = Textures.GBufferNormal;
			ImagesParams[2].TextureToVisualise = Textures.GBufferDS;
			ImagesParams[3].TextureToVisualise = Textures.GBufferRM;
			ImagesParams[4].TextureToVisualise = Textures.GBufferRM;
			ImagesParams[5].TextureToVisualise = Textures.FinalAfterTonemap;

			ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_NONE;
			ImagesParams[1].ShaderParams.Flags = ShaderParameters::FLAG_NORMAL;
			ImagesParams[2].ShaderParams.Flags = ShaderParameters::FLAG_DEPTH;
			ImagesParams[3].ShaderParams.Flags = ShaderParameters::FLAG_R_ONLY;
			ImagesParams[4].ShaderParams.Flags = ShaderParameters::FLAG_G_ONLY;
			ImagesParams[5].ShaderParams.Flags = ShaderParameters::FLAG_NONE;

			ImagesParams[0].BindingFlags = TextureBindingFlags::AspectColour;
			ImagesParams[1].BindingFlags = TextureBindingFlags::AspectColour;
			ImagesParams[2].BindingFlags = TextureBindingFlags::AspectDepth;
			ImagesParams[3].BindingFlags = TextureBindingFlags::AspectColour;
			ImagesParams[4].BindingFlags = TextureBindingFlags::AspectColour;
			ImagesParams[5].BindingFlags = TextureBindingFlags::AspectColour;
			break;
		case EDeferredRenderVisualisationMode::GBufferAlbedo:
			ImagesParams[0].TextureToVisualise = Textures.GBufferAlbedo;
			break;
		case EDeferredRenderVisualisationMode::GBufferNormal:
			ImagesParams[0].TextureToVisualise = Textures.GBufferNormal;
			ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_NORMAL;
			break;
		case EDeferredRenderVisualisationMode::GBufferRoughness:
			ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_R_ONLY;
			ImagesParams[0].TextureToVisualise = Textures.GBufferRM;
			break;
		case EDeferredRenderVisualisationMode::GBufferMetallic:
			ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_G_ONLY;
			ImagesParams[0].TextureToVisualise = Textures.GBufferRM;
			break;
		case EDeferredRenderVisualisationMode::GBufferDepth:
			ImagesParams[0].ShaderParams.Flags = ShaderParameters::FLAG_DEPTH;
			ImagesParams[0].TextureToVisualise = Textures.GBufferDS;
			ImagesParams[0].BindingFlags = TextureBindingFlags::AspectDepth;
			break;
		case EDeferredRenderVisualisationMode::Velocity:
			ImagesParams[0].TextureToVisualise = Textures.Velocity;
			break;
		case EDeferredRenderVisualisationMode::Reflections:
			ImagesParams[0].TextureToVisualise = Textures.RTReflections;
			break;
		case EDeferredRenderVisualisationMode::RTGI:
			ImagesParams[0].TextureToVisualise = Textures.RTGI;
			break;
		default:
			ImagesParams[0].TextureToVisualise = Textures.GBufferAlbedo;
			break;
		}

		for (int i = 0; i < ImagesParamsCount; i++)
		{
			Dependencies.Read(ImagesParams[i].TextureToVisualise, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}

		Graph.AddPass("Debug Visualisation", RenderGraphPassType::Raster, Parameters, Dependencies, [ImagesParams, ImagesParamsCount, Size](RenderGraphContext& Context)
		{
			static GraphicsPipeline* Pipeline = nullptr;
			if (Pipeline == nullptr)
			{
				GraphicsPipelineDesc Desc;
				Desc.Name = "DebugVisualisation";
				Desc.rasterizerState.Cull = CullMode::No;
				Desc.blendState.RenderTargets = {
					RenderTargetBlendDesc(),
				};

				Desc.depthStencilState.DepthEnable = false;
				Desc.depthStencilState.DepthWriteMask = false;
				Desc.Bytecode = LoadCompiledShaderData("./PrecompiledShaders/Visualisation.csd");

				Pipeline = Context.Device->CreateGraphicsPipeline(Desc, Context.VulkanRenderPass);
			}

			Vector2 ViewportSize = ImagesParamsCount == 1 ? Size : Size / Vector2(3, 2);

			for (int i = 0; i < ImagesParamsCount; i++)
			{
				VisualisationImageParameters Params = ImagesParams[i];
				Params.ShaderParams.ViewportSize = Size;

				auto DescriptorSet = Context.GetDescriptorSet(Pipeline, 0);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 0, 0, Context.GetRenderGraphTexture(Params.TextureToVisualise).get(), Params.BindingFlags, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
				Context.Device->UpdateDescriptorSet(DescriptorSet, 1, 0, Context.Device->GetStaticSampler<TextureFilter2::Linear, TextureAddressMode::ClampToEdge>());

				Vector2 CurrentPos;
				CurrentPos.X = (i % 3) * ViewportSize.X;
				CurrentPos.Y = (i / 3) * ViewportSize.Y;

				Context.CommandBuffer->SetViewport(CurrentPos.X, CurrentPos.Y, ViewportSize.X, ViewportSize.Y, 0, 1);
				Context.CommandBuffer->SetScissor((i32)CurrentPos.X, (i32)CurrentPos.Y, (i32)ViewportSize.X, (i32)ViewportSize.Y);

				Context.CommandBuffer->BindGraphicsPipeline(Pipeline);
				Context.CommandBuffer->BindDescriptorSetsGraphics(Pipeline, 0, 1, &DescriptorSet);
				Context.CommandBuffer->PushConstantsGraphics(Pipeline, ShaderType::Pixel, 0, sizeof(Params.ShaderParams), &Params.ShaderParams);
				Context.CommandBuffer->Draw(3, 1, 0, 0);
			}
		});

		return Result;
	}

	void ShowDebugConsole()
	{
		static char buf[1024]{ 0 };
		static std::vector<UPtr<char>> CommandHistory;
		static std::vector<UPtr<char>> History;

		const auto RightAlignText = [](const char* Text)
		{
			float region_width = ImGui::GetContentRegionAvail().x;
			float text_width = ImGui::CalcTextSize(Text).x;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + region_width - text_width);
			ImGui::TextDisabled(Text);
		};

		// TODO: move to a proper place
		if (ImGui::Begin("Stats"))
		{
			ImGuiTableFlags flags = ImGuiTableFlags_Borders;

			for (const char* cpu_category : GetProfilerCategoryListCPU())
			{
				auto category = GetProfilerCategoryCPU(cpu_category);

				ImGui::Text(cpu_category);
				ImGui::SameLine();
				RightAlignText("CPU");
				if (ImGui::BeginTable(cpu_category, 2, flags))
				{
					for (auto counter : category)
					{
						ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
						ImGui::TableNextColumn(); ImGui::Text("%.2fms", counter->LastTime);
					}
					ImGui::EndTable();
				}
			}

			for (const char* gpu_category : GetProfilerCategoryListGPU())
			{
				auto category = GetProfilerCategoryGPU(gpu_category);

				ImGui::Text(gpu_category);
				ImGui::SameLine();
				RightAlignText("GPU");
				if (ImGui::BeginTable(gpu_category, 2, flags))
				{
					for (auto counter : category)
					{
						ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
						ImGui::TableNextColumn(); ImGui::Text("%.2fms", counter->LastTime);
					}
					ImGui::EndTable();
				}
			}

			for (const char* mem_category : GetProfilerCategoryListMemory())
			{
				auto category = GetProfilerCategoryMemory(mem_category);

				ImGui::Text(mem_category);
				ImGui::SameLine();
				RightAlignText("MEM");
				if (ImGui::BeginTable(mem_category, 2, flags))
				{
					for (auto counter : category)
					{
						double mem;
						const char* postfix = HumanizeBytes(counter->Memory, mem);

						ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
						ImGui::TableNextColumn(); ImGui::Text("%.2f%s", mem, postfix);
					}
					ImGui::EndTable();
				}
			}

			for (const char* cnt_category : GetProfilerCategoryListCounting())
			{
				auto category = GetProfilerCategoryCounting(cnt_category);

				ImGui::Text(cnt_category);
				ImGui::SameLine();
				RightAlignText("COUNT");
				if (ImGui::BeginTable(cnt_category, 2, flags))
				{
					for (auto counter : category)
					{
						ImGui::TableNextColumn(); ImGui::TextDisabled("%s", counter->Text);
						ImGui::TableNextColumn(); ImGui::Text("%lu", counter->Count);
					}
					ImGui::EndTable();
				}
			}

		}
		ImGui::End();

		static bool isPopupVisible = false;
		static bool isHistoryPopup = true;
		static int popupIndex = 0;
		ImVec2 PopupPos;

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
			}
			ImGui::EndChild();

			ImGui::Separator();

			if (!DebugConsoleFocus)
			{
				isPopupVisible = false;
				isHistoryPopup = true;
			}

			if (DebugConsoleFocus && isPopupVisible && ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				isPopupVisible = false;
				isHistoryPopup = true;
			}

			if (DebugConsoleFocus && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
				ImGui::SetKeyboardFocusHere();

			PopupPos = ImGui::GetCursorScreenPos();

			auto InputCallback = [](ImGuiInputTextCallbackData* data) -> int
			{
				if (data->EventFlag & ImGuiInputTextFlags_CallbackHistory)
				{
					if (data->EventKey == ImGuiKey_UpArrow) popupIndex--;
					if (data->EventKey == ImGuiKey_DownArrow) popupIndex++;
					
					if (data->BufTextLen == 0 && !isPopupVisible)
					{
						isHistoryPopup = true;
					}
				
					if (isHistoryPopup)
					{
						if (!CommandHistory.empty())
						{
							if (!isPopupVisible) popupIndex = (int)CommandHistory.size() - 1;
							isPopupVisible = true;

							popupIndex = std::clamp(popupIndex, 0, (int)CommandHistory.size() - 1);

							data->DeleteChars(0, data->BufTextLen);
							data->InsertChars(0, CommandHistory[popupIndex].get());
						}
					} else
					{
						auto CvarList = ConsoleVariableSystem::GetConsoleVariableList();

						if (!CvarList.empty())
						{
							if (!isPopupVisible) popupIndex = (int)CvarList.size() - 1;
							isPopupVisible = true;

							popupIndex = std::clamp(popupIndex, 0, (int)CvarList.size() - 1);

							data->DeleteChars(0, data->BufTextLen);
							data->InsertChars(0, CvarList[popupIndex]);
						}
					}
				}

				if (data->EventFlag & ImGuiInputTextFlags_CallbackCompletion)
				{
					if (data->BufTextLen == 0)
					{
						isPopupVisible = true;
						isHistoryPopup = false;
					}
				}

				return 0;
			};

			if (ImGui::InputText("Input", buf, 1024, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackAlways, InputCallback)
			    && strlen(buf) > 0)
			{
				char cmd[1024];
				snprintf(cmd, 1024, ">>> %s <<<", buf);

				CommandHistory.emplace_back(strdup(buf));
				History.emplace_back(strdup(cmd));
				History.emplace_back(ConsoleVariableSystem::RunConsoleCommand(buf));
				memset(buf, 0, 1024);

				isPopupVisible = false;
			}
		}
		ImGui::End();

		if (ImGui::Begin("Loaded Assets"))
		{
			auto& assets = AssetSystem::Get();
			if (ImGui::BeginTable("AssetsTable", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("Path");
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("RefCount");
				ImGui::TableHeadersRow();

				for (const auto& [path, data] : assets.LoadedAssets)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn(); ImGui::TextUnformatted(path.c_str());
					ImGui::TableNextColumn(); ImGui::TextUnformatted(data.Type ? data.Type->Name : "Unknown");
					ImGui::TableNextColumn(); ImGui::Text("%d", data.RefCount);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();

		if (isPopupVisible)
		{
			ImGuiWindowFlags popupFlags = 
				ImGuiWindowFlags_NoTitleBar            | 
				ImGuiWindowFlags_NoResize              |
				ImGuiWindowFlags_NoMove                |
				ImGuiWindowFlags_HorizontalScrollbar   |
				ImGuiWindowFlags_NoSavedSettings       |
				// ImGuiWindowFlags_NoBringToFrontOnFocus |
				ImGuiWindowFlags_NoFocusOnAppearing;

			bool isOpenDummy = true;

			ImVec2 popupSize = ImVec2(200, 100);
			ImColor selectedColor = ImColor(252, 186, 3, 255);
			ImColor notSelectedColor = ImColor(255, 255, 255, 255);

			ImGui::SetNextWindowPos(PopupPos - ImVec2(0, popupSize.y));
			ImGui::SetNextWindowSize(popupSize);
			ImGui::Begin("console_hisory", &isOpenDummy, popupFlags);
			ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
			ImGui::PushAllowKeyboardFocus(false);

			if (isHistoryPopup)
			{
				for (size_t i = 0; i < CommandHistory.size(); i++)
				{
					if (i == popupIndex)
						ImGui::SetScrollHereY();

					const auto& Str = CommandHistory[i];
					ImGui::TextColored(i == popupIndex ? selectedColor : notSelectedColor, "%s", Str.get());
				}
			} else
			{
				// autocomplete popup
				auto CvarList = ConsoleVariableSystem::GetConsoleVariableList();
				for (size_t i = 0; i < CvarList.size(); i++)
				{
					if (i == popupIndex)
						ImGui::SetScrollHereY();

					ImGui::TextColored(i == popupIndex ? selectedColor : notSelectedColor, "%s", CvarList[i]);
				}
			}

			ImGui::PopAllowKeyboardFocus();
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
					ImGui::SetTooltip("%s\nId: %i\n", Pass.Name, Pass.Id);

				ImVec2 text_size = ImGui::CalcTextSize(Pass.Name);
				ImVec2 text_pos = middle - text_size / 2;
				draw_list->AddText(text_pos, pass_text_colours[(int)Pass.Type], Pass.Name);
				current_pos.x += rect_size.x + margin;
			}

			for (const auto& Texture : Info.Textures)
			{
				current_line_pos.y += vertical_size + margin;
				const ImVec2 min = ImVec2(Texture.FirstUsage * rect_size.x + Texture.FirstUsage * margin, 0) + current_line_pos;
				const ImVec2 max = ImVec2((Texture.LastUsage + 1) * rect_size.x + Texture.LastUsage * margin, vertical_size) + current_line_pos;
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
						ImGui::TableNextColumn(); ImGui::Text("%s", TextureFormatGetInfo(Texture.Desc.Format).FriendlyName);
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
