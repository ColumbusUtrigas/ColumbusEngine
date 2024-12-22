#include "DebugUI.h"
#include "Scene/Project.h"
#include "Editor/CommonUI.h"
#include "Scene/AssetImport.h"

// Third party
#include <Lib/imgui/imgui.h>
#include <Lib/imgui/backends/imgui_impl_vulkan.h>
#include <Lib/imgui/backends/imgui_impl_sdl2.h>
#include <Lib/implot/implot.h>
#include <Lib/ImGuizmo/ImGuizmo.h>
#include <Lib/nativefiledialog/src/include/nfd.h>

// std
#include <unordered_map>

namespace Columbus::DebugUI
{

	static constexpr TextureFormat SwapchainFormat = TextureFormat::BGRA8SRGB;

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

		ImGui::CreateContext();
		ImPlotContext* PlotCtx = ImPlot::CreateContext();
		ImPlot::SetCurrentContext(PlotCtx);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ApplyDarkTheme();

		ImFontConfig lat, cyr, icons;
		cyr.MergeMode = true;
		icons.MergeMode = true;

		//const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		io.Fonts->AddFontFromFileTTF("./Data/A.ttf", 14.0f, &lat);
		io.Fonts->AddFontFromFileTTF("./Data/A.ttf", 14.0f, &cyr, io.Fonts->GetGlyphRangesCyrillic());
		//io.Fonts->AddFontFromFileTTF("./Data/FontAwesome5.ttf", 12.0f, &icons, icons_ranges);
		io.Fonts->Build();

		ImGui_ImplSDL2_InitForVulkan(Window->Window);
		SetupImguiForContext(Ctx);

		return Ctx;
	}

	void BeginFrame(Context* Ctx)
	{
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
							static char ImportPath[512]{};

							// TODO: should only be available if current project is set (?)
							// TODO: common pick path widget

							ImGui::InputText("Source Path", SourcePath, 512);
							ImGui::SameLine();
							if (ImGui::Button("..."))
							{
								char* Path = NULL;
								if (NFD_OpenDialog("gltf", NULL, &Path) == NFD_OKAY)
								{
									strcpy(SourcePath, Path);
								}
							}

							ImGui::InputText("Path", ImportPath, 512);
							ImGui::SameLine();
							if (ImGui::Button("...##2"))
							{
								char* SavePath = NULL;
								// TODO: ensure that save path is under the project folder
								if (NFD_SaveDialog("gltf", NULL, &SavePath) == NFD_OKAY)
								{
									strcpy(ImportPath, SavePath);
								}
							}

							ImGui::BeginDisabled((strlen(SourcePath) == 0) || (strlen(ImportPath) == 0));
							if (ImGui::Button("Import"))
							{
								ImGui::EndDisabled();

								// TODO: handle errors
								Assets::ImportLevel(SourcePath, ImportPath);

								// TODO: async (?) import
								// TODO: modal progress window that cannot be closed
								return true;
							}
							ImGui::EndDisabled();

							return false;
						});
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("New Level"))
					{
						// Do stuff
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Open Level..."))
					{
						// Do stuff
					}

					ImGui::Spacing();
					if (ImGui::MenuItem("Save Level"))
					{
						// Do stuff
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

	static void DrawObjectLeaf(EngineWorld& World, GameObject& Object)
	{
		char Label[256]{ 0 };
		snprintf(Label, 256, "%i", Object.Id);

		int flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		//if (Object.Children.size() == 1) flags |= ImGuiTreeNodeFlags_Leaf;

		Transform& Trans = Object.Trans;

		ImGui::PushID(Object.Id);
		Vector3 Euler = Trans.Rotation.Euler();

		if (ImGui::TreeNodeEx(Object.Name.c_str(), flags))
		{
			ImGui::Text("Id: %i", Object.Id);
			ImGui::Text("Mesh Primitives: %i", World.Meshes[Object.MeshId]->Primitives.size());
			ImGui::SliderFloat3("Position", (float*)&Trans.Position, -10, +10);
			ImGui::SliderFloat3("Rotation", (float*)&Euler, 0, 360);
			ImGui::SliderFloat4("Quat", (float*)&Trans.Rotation, 0, 360);
			ImGui::SliderFloat3("Scale", (float*)&Trans.Scale, 0, +10);

			GameObjectId Parent = Object.ParentId;
			if (ImGui::InputInt("Parent", &Parent))
			{
				World.ReparentGameObject(Object.Id, Parent);
			}

			Trans.Rotation = Quaternion(Euler);

			for (int Child : Object.Children)
			{
				DrawObjectLeaf(World, World.GameObjects[Child]);
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void ShowMeshesWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Mesh"))
		{
			for (int i = 0; i < World.GameObjects.size(); i++)
			{
				GameObject& Object = World.GameObjects[i];

				// begin with root nodes
				if (Object.ParentId == -1)
				{
					DrawObjectLeaf(World, Object);
				}
			}
		}
		ImGui::End();
	}

	void ShowDecalsWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Decal"))
		{
			for (int i = 0; i < (int)World.SceneGPU->Decals.size(); i++)
			{
				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);

				if (ImGui::CollapsingHeader(Label))
				{
					GPUDecal& Decal = World.SceneGPU->Decals[i];

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
		if (ImGui::Begin("Light"))
		{
			// TODO: more robust sytem, make a function in World to add/delete lights
			fixed_vector<int, 16> LightsToDelete;

			for (int i = 0; i < (int)World.SceneGPU->Lights.size(); i++)
			{
				GPULight& Light = World.SceneGPU->Lights[i];

				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);
				if (ImGui::CollapsingHeader(Label))
				{
					const char* LightTypes[] = {
						LightTypeToString(LightType::Directional),
						LightTypeToString(LightType::Point),
						LightTypeToString(LightType::Spot),
						LightTypeToString(LightType::Rectangle),
					};

					ImGui::Combo("Type", (int*)&Light.Type, LightTypes, (int)LightType::Count);
					ImGui::SliderFloat3("Position", (float*)&Light.Position, -500, +500);
					ImGui::SliderFloat3("Direction", (float*)&Light.Direction, -1, +1);
					ImGui::ColorPicker3("Colour", (float*)&Light.Color, ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);
					ImGui::SliderFloat("Range", (float*)&Light.Range, 1, 1000);
					ImGui::SliderFloat("Source Radius", (float*)&Light.SourceRadius, 0, 5);

					if (ImGui::Button("-"))
					{
						LightsToDelete.push_back(i);
					}
				}
				ImGui::PopID();
			}

			for (int LightId : LightsToDelete)
			{
				// TODO: think about cleaning up render resources for light source
				World.SceneGPU->Lights.erase(World.SceneGPU->Lights.begin() + LightId); // TODO: RemoveLight function
			}

			if (ImGui::Button("+"))
			{
				GPULight NewLight{ {}, {0,1,0,0}, {1,1,1,1}, LightType::Point, 100, 0 };
				World.SceneGPU->Lights.push_back(NewLight); // TODO: AddLight function
			}
		}
		ImGui::End();
	}

	void ShowMaterialsWindow(EngineWorld& World)
	{
		if (ImGui::Begin("Material"))
		{
			for (int i = 0; i < (int)World.SceneGPU->Materials.size(); i++)
			{
				ImGui::PushID(i);
				char Label[256]{ 0 };
				snprintf(Label, 256, "%i", i);

				if (ImGui::CollapsingHeader(Label))
				{
					Material& Mat = World.SceneGPU->Materials[i];

					ImGui::InputFloat4("Albedo Factor", (float*)&Mat.AlbedoFactor);
					ImGui::InputFloat4("Emissive Factor", (float*)&Mat.EmissiveFactor);

					ImGui::InputInt("Albedo", &Mat.AlbedoId);
					ImGui::InputInt("Normal", &Mat.NormalId);
					ImGui::InputInt("ORM", &Mat.OrmId);
					ImGui::InputInt("Emissive", &Mat.EmissiveId);

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

						for (auto& GO : World.GameObjects)
						{
							Min = Vector3::Min(Min, World.Meshes[GO.MeshId]->BoundingBox.Min);
							Max = Vector3::Max(Max, World.Meshes[GO.MeshId]->BoundingBox.Max);
						}

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

			static VkDescriptorSet PreviewImage = NULL;

			if (ImGui::Button("Generate UV2"))
			{
				GenerateAndPackLightmaps(World);
				UploadLightmapMeshesToGPU(World);

				// TODO: make imgui image preview work normally
				TextureVulkan* vktex = static_cast<TextureVulkan*>(World.Lightmaps.Atlas.Lightmap);
				PreviewImage = ImGui_ImplVulkan_AddTexture(vktex->_Sampler, vktex->_View, vktex->_Layout);
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
				ImGui::Image(PreviewImage, ImVec2(200, 200));
			}
		}
		ImGui::End();
	}

	void TextureWidget(Texture2* Texture, Vector2 Size, bool ForceInvalidate)
	{
		// TODO: cleanup
		static std::unordered_map<Texture2*, VkDescriptorSet> ImguiTexturesMap;

		// TODO: find a way to hook imgui to rendering properly
		if (!ImguiTexturesMap.contains(Texture) || ForceInvalidate)
		{
			TextureVulkan* vktex = static_cast<TextureVulkan*>(Texture);

			ImguiTexturesMap[Texture] = ImGui_ImplVulkan_AddTexture(vktex->_Sampler, vktex->_View, vktex->_Layout);
		}

		ImGui::Image(ImguiTexturesMap[Texture], ImVec2(Size.X, Size.Y));
	}

}
