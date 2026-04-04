#include "DebugUI.h"
#include "Scene/Project.h"
#include "Editor/CommonUI.h"
#include "Scene/AssetImport.h"
#include "Editor/FontAwesome.h"

// Third party
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/backends/imgui_impl_vulkan.h>
#include <Lib/imgui/backends/imgui_impl_sdl2.h>
#include <Lib/implot/implot.h>
#include <Lib/ImGuizmo/ImGuizmo.h>
#include <Lib/nativefiledialog/src/include/nfd.h>

// std
#include <unordered_map>
#include <filesystem>

namespace Columbus::DebugUI
{

	static constexpr TextureFormat SwapchainFormat = TextureFormat::BGRA8SRGB;

	struct RetiredImguiTexture
	{
		VkDescriptorSet Set = VK_NULL_HANDLE;
		VkImageView PreviewView = VK_NULL_HANDLE;
		VkSampler PreviewSampler = VK_NULL_HANDLE;
		int FramesLeft = MaxFramesInFlight;
	};

	struct ImguiTextureBinding
	{
		VkDescriptorSet Set = VK_NULL_HANDLE;
		VkImageView View = VK_NULL_HANDLE;
		VkSampler Sampler = VK_NULL_HANDLE;
		VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageView PreviewView = VK_NULL_HANDLE;
		VkSampler PreviewSampler = VK_NULL_HANDLE;
		TextureWidgetSettings Settings;
	};

	static std::vector<RetiredImguiTexture> GRetiredImguiTextures;
	static std::unordered_map<Texture2*, ImguiTextureBinding> GImguiTexturesMap;
	static VkDescriptorSet GLightmapPreviewImage = VK_NULL_HANDLE;
	static SPtr<DeviceVulkan> GDebugUIDevice;

	static bool operator==(const TextureWidgetSettings& A, const TextureWidgetSettings& B)
	{
		return A.ShowRed == B.ShowRed
			&& A.ShowGreen == B.ShowGreen
			&& A.ShowBlue == B.ShowBlue
			&& A.ShowAlpha == B.ShowAlpha
			&& A.ShowCheckerboard == B.ShowCheckerboard
			&& A.MipLevel == B.MipLevel
			&& A.Zoom == B.Zoom;
	}

	static void DestroyRetiredPreviewResources(const RetiredImguiTexture& Retired)
	{
		if (!GDebugUIDevice)
			return;

		if (Retired.PreviewView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(GDebugUIDevice->_Device, Retired.PreviewView, nullptr);
		}
		if (Retired.PreviewSampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(GDebugUIDevice->_Device, Retired.PreviewSampler, nullptr);
		}
	}

	static void RetireImguiTexture(VkDescriptorSet Set, VkImageView PreviewView = VK_NULL_HANDLE, VkSampler PreviewSampler = VK_NULL_HANDLE)
	{
		if (Set != VK_NULL_HANDLE)
		{
			GRetiredImguiTextures.push_back({ Set, PreviewView, PreviewSampler, MaxFramesInFlight });
		}
	}

	static void FlushRetiredImguiTextures()
	{
		for (size_t Index = 0; Index < GRetiredImguiTextures.size();)
		{
			RetiredImguiTexture& Retired = GRetiredImguiTextures[Index];
			Retired.FramesLeft--;

			if (Retired.FramesLeft <= 0)
			{
				ImGui_ImplVulkan_RemoveTexture(Retired.Set);
				DestroyRetiredPreviewResources(Retired);
				GRetiredImguiTextures.erase(GRetiredImguiTextures.begin() + Index);
			}
			else
			{
				Index++;
			}
		}
	}

	static void ClearImguiTextureBindings()
	{
		for (auto& [Texture, Binding] : GImguiTexturesMap)
		{
			(void)Texture;
			if (Binding.Set != VK_NULL_HANDLE)
			{
				ImGui_ImplVulkan_RemoveTexture(Binding.Set);
			}
			if (Binding.PreviewView != VK_NULL_HANDLE && Binding.PreviewView != Binding.View)
			{
				vkDestroyImageView(GDebugUIDevice->_Device, Binding.PreviewView, nullptr);
			}
			if (Binding.PreviewSampler != VK_NULL_HANDLE && Binding.PreviewSampler != Binding.Sampler)
			{
				vkDestroySampler(GDebugUIDevice->_Device, Binding.PreviewSampler, nullptr);
			}
		}
		GImguiTexturesMap.clear();

		if (GLightmapPreviewImage != VK_NULL_HANDLE)
		{
			ImGui_ImplVulkan_RemoveTexture(GLightmapPreviewImage);
			GLightmapPreviewImage = VK_NULL_HANDLE;
		}

		for (RetiredImguiTexture& Retired : GRetiredImguiTextures)
		{
			if (Retired.Set != VK_NULL_HANDLE)
			{
				ImGui_ImplVulkan_RemoveTexture(Retired.Set);
			}
			DestroyRetiredPreviewResources(Retired);
		}
		GRetiredImguiTextures.clear();
	}

	static void ApplyDarkTheme()
	{
		ImGuiStyle& Style = ImGui::GetStyle();
		ImGui::StyleColorsDark(&Style);
		Style.FrameRounding = 3.0f;
		Style.WindowRounding = 0.0f;
		Style.ScrollbarRounding = 3.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.75f, 0.75f, 0.75f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
	}

	static void SetupImguiForContext(Context* Ctx)
	{
		SPtr<DeviceVulkan> Device = Ctx->Window->Device;
		SwapchainVulkan* Swapchain = Ctx->Window->Swapchain;

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
			AttachmentDesc { AttachmentType::Color, AttachmentLoadOp::Load, SwapchainFormat }
		};

		Ctx->InternalRenderPass = Device->CreateRenderPass(Attachments);

		ImGui_ImplVulkan_Init(&imguiVk, Ctx->InternalRenderPass);
		{
			auto CommandBuffer = Device->CreateCommandBufferShared();
			CommandBuffer->Begin();
			ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->_CmdBuf);
			CommandBuffer->End();
			Device->Submit(CommandBuffer.get());
			Device->QueueWaitIdle();
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	Context* Create(WindowVulkan* Window)
	{
		Context* Ctx = new Context();
		Ctx->Window = Window;
		Ctx->Graph = new RenderGraph(Window->Device, nullptr);
		GDebugUIDevice = Window->Device;

		ImGui::CreateContext();
		ImPlotContext* PlotCtx = ImPlot::CreateContext();
		ImPlot::SetCurrentContext(PlotCtx);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ApplyDarkTheme();

		ImFontConfig lat, cyr, icons;
		cyr.MergeMode = true;
		icons.MergeMode = true;

		const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		std::string fontAwesomePath = GCurrentProject->DataPath + "/FontAwesome5.ttf";

		io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 14.0f, &lat);
		io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 14.0f, &cyr, io.Fonts->GetGlyphRangesCyrillic());
		io.Fonts->AddFontFromFileTTF(fontAwesomePath.c_str(), 12.0f, &icons, icons_ranges);
		io.Fonts->Build();

		ImGui_ImplSDL2_InitForVulkan(Window->Window);
		SetupImguiForContext(Ctx);

		return Ctx;
	}

	void BeginFrame(Context* Ctx)
	{
		FlushRetiredImguiTextures();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.DisplaySize = { (float)Ctx->Window->Size.X, (float)Ctx->Window->Size.Y };

		if (io.Fonts->IsBuilt())
		{
			ImGui::NewFrame();
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL2_NewFrame(Ctx->Window->Window);
		}
	}

	void ProcessInputSDL(Context* Ctx, SDL_Event* Event)
	{
		ImGui_ImplSDL2_ProcessEvent(Event);
	}

	RenderResult Render(Context* Ctx, VkSemaphore WaitSemaphore)
	{
		RenderGraphTextureRef UITexture;

		iVector2 OutputSize = Ctx->Window->Size;
		RenderGraph& Graph = *Ctx->Graph;

		Graph.Clear();

		{
			TextureDesc2 UIFinalDesc;
			UIFinalDesc.Format = SwapchainFormat;
			UIFinalDesc.Width = (u32)OutputSize.X;
			UIFinalDesc.Height = (u32)OutputSize.Y;
			UIFinalDesc.Usage = TextureUsage::RenderTargetColor;
			UITexture = Graph.CreateTexture(UIFinalDesc, "UI");

			RenderPassParameters Parameters;
			Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Clear, UITexture, {} };
			RenderPassDependencies Dependencies(Graph.Allocator);

			// empty pass just to clear the texture
			Graph.AddPass("ClearDebugUI", RenderGraphPassType::Raster, Parameters, Dependencies, [](RenderGraphContext& Context)
			{}
			);

			// then set the internal render pass to use ImGUIs internal pass
			Parameters.ExternalRenderPass = Ctx->InternalRenderPass;

			Graph.AddPass("DebugUI", RenderGraphPassType::Raster, Parameters, Dependencies, [UITexture](RenderGraphContext& Context)
			{
				ImGui::Render();
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context.CommandBuffer->_CmdBuf);
			});
		}

		RenderGraphExecuteParameters RGParameters;
		RGParameters.DefaultViewportSize = OutputSize;
		RGParameters.WaitSemaphore = WaitSemaphore;
		RenderGraphExecuteResults RGResults = Graph.Execute(RGParameters);

		RenderResult Result;
		Result.FinishSemaphore = RGResults.FinishSemaphore;
		Result.ResultTexture = Graph.GetTextureAfterExecution(UITexture);

		return Result;
	}

	RenderResult RenderOverlay(Context* Ctx, VkSemaphore WaitSemaphore, SPtr<Texture2> OverlayTexture)
	{
		RenderGraphTextureRef UITexture;

		iVector2 OutputSize = Ctx->Window->Size;
		RenderGraph& Graph = *Ctx->Graph;

		Graph.Clear();

		{
			UITexture = Graph.RegisterExternalTexture(OverlayTexture, "UI");

			RenderPassParameters Parameters;
			Parameters.ColorAttachments[0] = RenderPassAttachment{ AttachmentLoadOp::Load, UITexture, {} };
			Parameters.ExternalRenderPass = Ctx->InternalRenderPass;
			RenderPassDependencies Dependencies(Graph.Allocator);

			Graph.AddPass("DebugUI", RenderGraphPassType::Raster, Parameters, Dependencies, [OverlayTexture](RenderGraphContext& Context)
			{
				ImGui::Render();
				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context.CommandBuffer->_CmdBuf);
			});
		}

		RenderGraphExecuteParameters RGParameters;
		RGParameters.DefaultViewportSize = OutputSize;
		RGParameters.WaitSemaphore = WaitSemaphore;
		RenderGraphExecuteResults RGResults = Graph.Execute(RGParameters);

		RenderResult Result;
		Result.FinishSemaphore = RGResults.FinishSemaphore;
		Result.ResultTexture = Graph.GetTextureAfterExecution(UITexture);

		return Result;
	}

	void EndFrame(Context* Ctx)
	{
	}

	void Destroy(Context* Ctx)
	{
		if (Ctx == nullptr)
		{
			return;
		}

		Ctx->Window->Device->QueueWaitIdle();
		ClearImguiTextureBindings();
		GDebugUIDevice.reset();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();

		if (ImPlot::GetCurrentContext() != nullptr)
		{
			ImPlot::DestroyContext();
		}
		if (ImGui::GetCurrentContext() != nullptr)
		{
			ImGui::DestroyContext();
		}

		if (Ctx->InternalRenderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(Ctx->Window->Device->_Device, Ctx->InternalRenderPass, nullptr);
			Ctx->InternalRenderPass = VK_NULL_HANDLE;
		}

		delete Ctx->Graph;
		delete Ctx;
	}

	// TODO: these things are not "debug ui" anymore, but an editor, so it should go there instead
	static void DrawMainMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				{
					ImGui::Spacing();
					if (ImGui::MenuItem("New Project..."))
					{
						Editor::ShowModalWindow("New Project", []()->bool
						{
							static char Name[256]{};
							static char Path[512]{};

							ImGui::InputText("Name", Name, 256);
							ImGui::InputText("Path", Path, 512);
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								char* SavePath = NULL;
								if (NFD_PickFolder(NULL, &SavePath) == NFD_OKAY)
								{
									strcpy(Path, SavePath);
								}
							}

							ImGui::BeginDisabled((strlen(Name) == 0) || (strlen(Path) == 0));
							if (ImGui::Button("Create"))
							{
								GCurrentProject = EngineProject::CreateProject(Name, Path);
								ImGui::EndDisabled();
								return true;
							}
							ImGui::EndDisabled();

							return false;
						});
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Open Project..."))
					{
						char* Path;
						if (NFD_OpenDialog("json", NULL, &Path) == NFD_OKAY)
						{
							GCurrentProject = EngineProject::LoadProject(Path);
						}
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Import Level..."))
					{
						Editor::ShowModalWindow("Import Level", []()->bool
						{
							static char SourcePath[512]{};


							auto SourceFolderPath = AssetSystem::Get().SourceDataPath;
							std::filesystem::path SourceRelativePath;

							ImGui::InputText("Source Path", SourcePath, 512);
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								char* Path = NULL;
								if (NFD_OpenDialog("gltf,glb", NULL, &Path) == NFD_OKAY)
								{
									strcpy(SourcePath, Path);
									if (!AssetSystem::Get().IsPathInSourceFolder(SourcePath))
									{
										char ErrorBuf[4096]{ 0 };
										snprintf(ErrorBuf, 4096, "Source asset has to be in the data source folder");

										Editor::ShowMessageBox("Asset Import Error", ErrorBuf, {});
										Log::Error(ErrorBuf);

										memset(SourcePath, 0, 512);
									}
								}
							}

							ImGui::BeginDisabled((strlen(SourcePath) == 0));
							if (ImGui::Button("Import"))
							{
								auto SourcePathRel = AssetSystem::Get().MakePathRelativeToSourceFolder(SourcePath);
								auto DestPathAbs = std::filesystem::path(AssetSystem::Get().DataPath) / SourcePathRel;
								auto DestPathFolder = DestPathAbs.parent_path();

								std::filesystem::create_directories(DestPathFolder); // mimic the same folder structure as the source

								ImGui::EndDisabled();

								Assets::ImportLevel(SourcePath, DestPathAbs.string().c_str());

								return true;
							}
							ImGui::EndDisabled();

							return false;
						});
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("New Level"))
					{
						GCurrentLevelPath = "";
						GCurrentProject->World->ClearWorld();
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Open Level..."))
					{
						char* path = nullptr;
						if (NFD_OpenDialog("clvl", NULL, &path) == NFD_OKAY)
						{
							HLevel* level = GCurrentProject->World->LoadLevelCLVL(path);
							GCurrentProject->World->AddLevel(level);
						}
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Save Level"))
					{
						if (!GCurrentProject || !GCurrentProject->World)
						{
							Log::Error("Can't save the level, no project is loaded or world doesn't exist");
						}

						if (GCurrentProject && GCurrentProject->World)
						{
							if (GCurrentLevelPath.empty())
							{
								std::filesystem::path LevelsDefaultPath = std::filesystem::path(GCurrentProject->BasePath + "/Data/Levels/");

								char* Path = NULL;
								nfdresult_t result = NFD_SaveDialog("clvl", LevelsDefaultPath.make_preferred().string().c_str(), (char**)&Path);
								if (result == NFD_OKAY)
								{
									std::string PathStr = Path;
									if (!PathStr.ends_with(".clvl"))
									{
										PathStr += ".clvl";
									}

									GCurrentLevelPath = PathStr;
									GCurrentProject->World->SaveWorldLevel(PathStr.c_str());
								}
							}
							else
							{
								GCurrentProject->World->SaveWorldLevel(GCurrentLevelPath.c_str());
							}
						}
					}
				}

				ImGui::Spacing();
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void DrawMainLayout()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		if (ImGui::Begin("MainLayout", nullptr, window_flags))
		{
			//DrawToolbar();

			ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
			ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

			ImGui::PopStyleVar(3);
			DrawMainMenu();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
	}

	void ShowProjectSettingsWindow()
	{
		if (ImGui::Begin("Project Settings"))
		{
			if (GCurrentProject == nullptr)
			{
				ImGui::Text("No current project available");
			}
			else
			{
				ImGui::InputText("Project Name", (char*)GCurrentProject->ProjectName.c_str(), GCurrentProject->ProjectName.size(), ImGuiInputTextFlags_ReadOnly);
				ImGui::InputText("Base Path", (char*)GCurrentProject->BasePath.c_str(), GCurrentProject->BasePath.size(), ImGuiInputTextFlags_ReadOnly);
			}
		}
		ImGui::End();
	}

	// mutates View
	void ShowScreenshotSaveWindow(RenderView& View)
	{
		if (ImGui::Begin("Screenshot"))
		{
			static bool HDR = false;

			if (ImGui::Button("Take screenshot"))
			{
				char* ScreenshotPath = NULL;

				if (HDR)
				{
					if (NFD_SaveDialog("exr", NULL, &ScreenshotPath) == NFD_OKAY)
						View.ScreenshotPath = ScreenshotPath;
				}
				else
				{
					if (NFD_SaveDialog("png", NULL, &ScreenshotPath) == NFD_OKAY)
						View.ScreenshotPath = ScreenshotPath;
				}

				View.ScreenshotHDR = HDR;
			}

			ImGui::Checkbox("HDR", &HDR);
		}
		ImGui::End();
	}

	void ShowMeshesWindow(EngineWorld& World)
	{
	}

	void ShowDecalsWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Decal"))
		{
			for (int i = 0; i < (int)World.SceneGPU->Decals.Size(); i++)
			{
				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);

				if (ImGui::CollapsingHeader(Label))
				{
					GPUDecal Decal = World.SceneGPU->Decals.Data()[i];

					Vector3 Position = Decal.Model.GetColumn(3).XYZ();
					Vector3 Scale = Vector3(Decal.Model.M[0][0], Decal.Model.M[1][1], Decal.Model.M[2][2]);

					ImGui::SliderFloat3("Position", (float*)&Position, -500, +500);
					ImGui::SliderFloat3("Scale", (float*)&Scale, 1, 500);

					Matrix Model;
					Model.Scale(Scale);
					Model.Translate(Position);

					Decal.Model = Model;
					Decal.ModelInverse = Model.GetInverted();

					World.MainView.DebugRender.AddBox(Model, Vector4(1, 1, 1, 0.1f));
				}

				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void ShowLightsWindow(EngineWorld& World)
	{
	}

	void ShowMaterialsWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Material"))
		{
			for (int i = 0; i < (int)World.SceneGPU->Materials.Size(); i++)
			{
				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);

				if (ImGui::CollapsingHeader(Label))
				{
					Material& Mat = World.SceneGPU->Materials[i];

					ImGui::InputFloat4("Albedo Factor", (float*)&Mat.AlbedoFactor);
					ImGui::InputFloat4("Emissive Factor", (float*)&Mat.EmissiveFactor);

					/*ImGui::InputInt("Albedo", &Mat.AlbedoId);
					ImGui::InputInt("Normal", &Mat.NormalId);
					ImGui::InputInt("ORM", &Mat.OrmId);
					ImGui::InputInt("Emissive", &Mat.EmissiveId);*/

					ImGui::SliderFloat("Roughness", &Mat.Roughness, 0, 1);
					ImGui::SliderFloat("Metallic", &Mat.Metallic, 0, 1);
				}

				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void ShowIrradianceWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Irradiance Volume"))
		{
			for (int i = 0; i < (int)World.SceneGPU->IrradianceVolumes.size(); i++)
			{
				IrradianceVolume& Volume = World.SceneGPU->IrradianceVolumes[i];

				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);

				if (ImGui::CollapsingHeader(Label))
				{
					ImGui::SliderFloat3("Position", (float*)&Volume.Position, -10, 10);
					ImGui::SliderFloat3("Extent", (float*)&Volume.Extent, -10, 10);
					ImGui::SliderInt3("Count", (int*)&Volume.ProbesCount, 2, 8);
					ImGui::SliderFloat3("TestPoint", (float*)&Volume.TestPoint, -5, 5);

					if (ImGui::Button("To World Bounds"))
					{
						Vector3 Min(999999);
						Vector3 Max(-999999);

						assert(false);

						/*for (auto& GO : World.GameObjects)
						{
							Min = Vector3::Min(Min, World.Meshes[GO.MeshId]->BoundingBox.Min);
							Max = Vector3::Max(Max, World.Meshes[GO.MeshId]->BoundingBox.Max);
						}*/

						Volume.Position = (Min + Max) / 2;
						Volume.Extent = (Max - Min);
					}

					Matrix ViewMat = World.MainView.CameraCur.GetViewMatrix().GetTransposed();
					Matrix ProjMat = World.MainView.CameraCur.GetProjectionMatrix().GetTransposed();

					Matrix Mat;
					Mat.Scale(Volume.Extent);
					Mat.Translate(Volume.Position);
					Mat.Transpose();

					Vector3 Bounds[2]{
						-Vector3(0.5f),
						+Vector3(0.5f),
					};

					// view projection
					ImGuizmo::Manipulate(&ViewMat.M[0][0], &ProjMat.M[0][0], ImGuizmo::OPERATION::BOUNDS, ImGuizmo::MODE::LOCAL, &Mat.M[0][0],
						nullptr, nullptr, (float*)&Bounds, nullptr);

					//Volume.Extent = (Bounds[1] - Bounds[0]);

					Vector3 Position, Euler, Scale;
					Mat.Transpose();
					Mat.DecomposeTransform(Position, Euler, Scale);

					if (Scale != Vector3(1))
					{
						int asd = 123;
					}

					Volume.Position = Position;
					Volume.Extent = Scale;

					// bounding
					if (0)
					{
						Matrix Transform;
						Transform.Scale(World.SceneGPU->IrradianceVolumes[i].Extent);
						Transform.Translate(World.SceneGPU->IrradianceVolumes[i].Position);
						World.MainView.DebugRender.AddBox(Transform, Vector4(1, 1, 1, 0.1f));
					}
				}

				ImGui::PopID();
			}
		}
		ImGui::End();
	}

	void ShowLightmapWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Lightmap"))
		{
			ImGui::InputInt("Samples", &World.Lightmaps.BakingSettings.RequestedSamples);
			ImGui::InputInt("Bounces", &World.Lightmaps.BakingSettings.Bounces);
			ImGui::InputInt("Samples per frame", &World.Lightmaps.BakingSettings.SamplesPerFrame);

			if (ImGui::Button("Generate UV2"))
			{
				GenerateAndPackLightmaps(World);
				UploadLightmapMeshesToGPU(World);

				// TODO: make imgui image preview work normally
				TextureVulkan* vktex = static_cast<TextureVulkan*>(World.Lightmaps.Atlas.Lightmap);
				if (GLightmapPreviewImage != VK_NULL_HANDLE)
				{
					ImGui_ImplVulkan_RemoveTexture(GLightmapPreviewImage);
				}
				GLightmapPreviewImage = ImGui_ImplVulkan_AddTexture(vktex->_Sampler, vktex->_View, vktex->_Layout);
			}

			if (ImGui::Button("Bake"))
			{
				World.Lightmaps.BakingRequested = true;
				World.Lightmaps.BakingData.AccumulatedSamples = 0;
			}

			if (World.Lightmaps.BakingRequested)
			{
				ImGui::ProgressBar((float)World.Lightmaps.BakingData.AccumulatedSamples / World.Lightmaps.BakingSettings.RequestedSamples);
			}

			if (World.Lightmaps.Atlas.Lightmap != nullptr)
			{
				ImGui::Image(GLightmapPreviewImage, ImVec2(200, 200));
			}
		}
		ImGui::End();
	}

	static VkComponentSwizzle ToChannelSwizzle(bool bEnabled, VkComponentSwizzle Channel)
	{
		return bEnabled ? Channel : VK_COMPONENT_SWIZZLE_ZERO;
	}

	static void BuildPreviewBinding(TextureVulkan* vktex, ImguiTextureBinding& Binding, const TextureWidgetSettings& Settings)
	{
		Binding.PreviewView = VK_NULL_HANDLE;
		Binding.PreviewSampler = VK_NULL_HANDLE;

		const TextureDesc2& Desc = vktex->GetDesc();
		const int ClampedMipLevel = std::clamp(Settings.MipLevel, 0, (int)Desc.Mips - 1);
		const bool bIdentityPreview =
			ClampedMipLevel == 0 &&
			Settings.ShowRed &&
			Settings.ShowGreen &&
			Settings.ShowBlue &&
			Settings.ShowAlpha;

		if (bIdentityPreview)
		{
			Binding.PreviewView = vktex->_View;
			Binding.PreviewSampler = vktex->_Sampler;
			return;
		}

		VkImageViewCreateInfo ViewInfo{};
		ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewInfo.image = vktex->_Image;
		ViewInfo.viewType = TextureTypeToViewTypeVk(Desc.Type);
		ViewInfo.format = TextureFormatToVK(Desc.Format);

		const bool bAlphaOnly = Settings.ShowAlpha && !Settings.ShowRed && !Settings.ShowGreen && !Settings.ShowBlue;
		if (bAlphaOnly)
		{
			ViewInfo.components.r = VK_COMPONENT_SWIZZLE_A;
			ViewInfo.components.g = VK_COMPONENT_SWIZZLE_A;
			ViewInfo.components.b = VK_COMPONENT_SWIZZLE_A;
			ViewInfo.components.a = VK_COMPONENT_SWIZZLE_ONE;
		}
		else
		{
			ViewInfo.components.r = ToChannelSwizzle(Settings.ShowRed, VK_COMPONENT_SWIZZLE_R);
			ViewInfo.components.g = ToChannelSwizzle(Settings.ShowGreen, VK_COMPONENT_SWIZZLE_G);
			ViewInfo.components.b = ToChannelSwizzle(Settings.ShowBlue, VK_COMPONENT_SWIZZLE_B);
			ViewInfo.components.a = Settings.ShowAlpha ? VK_COMPONENT_SWIZZLE_A : VK_COMPONENT_SWIZZLE_ONE;
		}

		ViewInfo.subresourceRange.aspectMask = TextureFormatToAspectMaskVk(Desc.Format);
		ViewInfo.subresourceRange.baseMipLevel = ClampedMipLevel;
		ViewInfo.subresourceRange.levelCount = 1;
		ViewInfo.subresourceRange.baseArrayLayer = 0;
		ViewInfo.subresourceRange.layerCount = Desc.ArrayLayers;
		VK_CHECK(vkCreateImageView(GDebugUIDevice->_Device, &ViewInfo, nullptr, &Binding.PreviewView));

		VkSamplerCreateInfo SamplerInfo{};
		SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		SamplerInfo.magFilter = TextureFilterToVk(Desc.MagFilter);
		SamplerInfo.minFilter = TextureFilterToVk(Desc.MinFilter);
		SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		SamplerInfo.addressModeU = TextureAddressModeToVk(Desc.AddressU);
		SamplerInfo.addressModeV = TextureAddressModeToVk(Desc.AddressV);
		SamplerInfo.addressModeW = TextureAddressModeToVk(Desc.AddressW);
		SamplerInfo.mipLodBias = 0.0f;
		SamplerInfo.anisotropyEnable = VK_FALSE;
		SamplerInfo.maxAnisotropy = 1.0f;
		SamplerInfo.compareEnable = VK_FALSE;
		SamplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		SamplerInfo.minLod = (float)ClampedMipLevel;
		SamplerInfo.maxLod = (float)ClampedMipLevel;
		SamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		SamplerInfo.unnormalizedCoordinates = VK_FALSE;
		VK_CHECK(vkCreateSampler(GDebugUIDevice->_Device, &SamplerInfo, nullptr, &Binding.PreviewSampler));
	}

	void ReleaseTextureWidget(Texture2* Texture)
	{
		if (Texture == nullptr)
			return;

		auto It = GImguiTexturesMap.find(Texture);
		if (It == GImguiTexturesMap.end())
			return;

		ImguiTextureBinding& Binding = It->second;
		if (Binding.Set != VK_NULL_HANDLE)
		{
			const VkImageView RetiredView = (Binding.PreviewView != Binding.View) ? Binding.PreviewView : VK_NULL_HANDLE;
			const VkSampler RetiredSampler = (Binding.PreviewSampler != Binding.Sampler) ? Binding.PreviewSampler : VK_NULL_HANDLE;
			RetireImguiTexture(Binding.Set, RetiredView, RetiredSampler);
		}

		GImguiTexturesMap.erase(It);
	}

	void TextureWidget(Texture2* Texture, Vector2 Size, const TextureWidgetSettings& Settings, bool ForceInvalidate)
	{
		if (Texture == nullptr)
		{
			ImGui::Dummy(ImVec2(Size.X, Size.Y));
			return;
		}

		TextureVulkan* vktex = static_cast<TextureVulkan*>(Texture);
		ImguiTextureBinding& Binding = GImguiTexturesMap[Texture];

		const bool bDescriptorChanged =
			Binding.Set == VK_NULL_HANDLE ||
			Binding.View != vktex->_View ||
			Binding.Sampler != vktex->_Sampler ||
			Binding.Layout != vktex->_Layout ||
			!(Binding.Settings == Settings);

		if (ForceInvalidate || bDescriptorChanged)
		{
			if (Binding.Set != VK_NULL_HANDLE)
			{
				const VkImageView RetiredView = (Binding.PreviewView != Binding.View) ? Binding.PreviewView : VK_NULL_HANDLE;
				const VkSampler RetiredSampler = (Binding.PreviewSampler != Binding.Sampler) ? Binding.PreviewSampler : VK_NULL_HANDLE;
				RetireImguiTexture(Binding.Set, RetiredView, RetiredSampler);
			}

			BuildPreviewBinding(vktex, Binding, Settings);

			Binding.Set = ImGui_ImplVulkan_AddTexture(
				Binding.PreviewSampler != VK_NULL_HANDLE ? Binding.PreviewSampler : vktex->_Sampler,
				Binding.PreviewView != VK_NULL_HANDLE ? Binding.PreviewView : vktex->_View,
				vktex->_Layout);
			Binding.View = vktex->_View;
			Binding.Sampler = vktex->_Sampler;
			Binding.Layout = vktex->_Layout;
			Binding.Settings = Settings;
		}

		const int MipLevel = std::clamp(Settings.MipLevel, 0, (int)Texture->GetDesc().Mips - 1);
		const float Zoom = std::max(0.0625f, Settings.Zoom);
		const ImVec2 ImageSize(
			(float)Math::Max(1u, Texture->GetDesc().Width >> MipLevel) * Zoom,
			(float)Math::Max(1u, Texture->GetDesc().Height >> MipLevel) * Zoom);

		if (Settings.ShowCheckerboard)
		{
			const ImVec2 Start = ImGui::GetCursorScreenPos();
			const ImVec2 End(Start.x + ImageSize.x, Start.y + ImageSize.y);
			ImDrawList* DrawList = ImGui::GetWindowDrawList();
			const int CellSize = 12;
			const ImVec2 ClipMin = ImGui::GetWindowPos();
			const ImVec2 ClipMax(ClipMin.x + ImGui::GetWindowSize().x, ClipMin.y + ImGui::GetWindowSize().y);
			const float VisibleMinX = std::max(Start.x, ClipMin.x);
			const float VisibleMaxX = std::min(End.x, ClipMax.x);
			const float VisibleMinY = std::max(Start.y, ClipMin.y);
			const float VisibleMaxY = std::min(End.y, ClipMax.y);

			if (VisibleMinX < VisibleMaxX && VisibleMinY < VisibleMaxY)
			{
				const int StartXi = (int)floorf(Start.x);
				const int StartYi = (int)floorf(Start.y);
				const int VisibleMinXi = (int)floorf(VisibleMinX);
				const int VisibleMaxXi = (int)ceilf(VisibleMaxX);
				const int VisibleMinYi = (int)floorf(VisibleMinY);
				const int VisibleMaxYi = (int)ceilf(VisibleMaxY);
				const int FirstCellX = StartXi + ((VisibleMinXi - StartXi) / CellSize) * CellSize;
				const int FirstCellY = StartYi + ((VisibleMinYi - StartYi) / CellSize) * CellSize;

				DrawList->PushClipRect(ImVec2(VisibleMinX, VisibleMinY), ImVec2(VisibleMaxX, VisibleMaxY), true);
				for (int Y = FirstCellY; Y < VisibleMaxYi; Y += CellSize)
				{
					for (int X = FirstCellX; X < VisibleMaxXi; X += CellSize)
					{
						const int Cell = (((X - StartXi) / CellSize) + ((Y - StartYi) / CellSize)) & 1;
						const ImU32 Color = Cell ? IM_COL32(74, 74, 74, 255) : IM_COL32(110, 110, 110, 255);
						const int X1 = X;
						const int Y1 = Y;
						const int X2 = std::min(X + CellSize, VisibleMaxXi);
						const int Y2 = std::min(Y + CellSize, VisibleMaxYi);
						DrawList->AddRectFilled(ImVec2((float)X1, (float)Y1), ImVec2((float)X2, (float)Y2), Color);
					}
				}
				DrawList->PopClipRect();
			}
		}

		ImGui::Image(Binding.Set, ImageSize);
	}

	void TextureWidget(Texture2* Texture, Vector2 Size, bool ForceInvalidate)
	{
		TextureWidgetSettings Settings;
		TextureWidget(Texture, Size, Settings, ForceInvalidate);
	}

}
