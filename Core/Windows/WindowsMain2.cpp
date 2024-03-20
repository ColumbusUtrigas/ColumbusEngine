#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Editor/CommonUI.h"
#include "Graphics/Camera.h"
#include "Graphics/GPUScene.h"
#include "Graphics/IrradianceVolume.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/TextureVulkan.h"
#include "Graphics/Window.h"
#include "Input/Events.h"
#include "Input/Input.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Geometry.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "Scene/Transform.h"
#include "Lib/imgui/imgui.h"
#include "Lib/imgui/backends/imgui_impl_vulkan.h"
#include "Lib/imgui/backends/imgui_impl_sdl2.h"
#include "Lib/implot/implot.h"
#include "Lib/ImGuizmo/ImGuizmo.h"
#include "System/File.h"
#include <vulkan/vulkan.h>
#include <Core/Core.h>
#include <Core/CVar.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/RenderPasses/RenderPasses.h>
#include <Graphics/Lightmaps.h>
#include <Graphics/World.h>
#include <Profiling/Profiling.h>
#include <Math/Box.h>
#include <Graphics/DebugUI.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <Graphics/Vulkan/InstanceVulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>

#include <vector>
#include <algorithm>
#include <queue>

#include <Lib/nativefiledialog/src/include/nfd.h>

using namespace Columbus;

ConsoleVariable<int> render_cvar("r.Render", "0 - Deferred, 1 - PathTraced, default - 0", 0);
ConsoleVariable<float> CVar_CameraSpeed("CameraSpeed", "", 10);
ConsoleVariable<bool> CVar_Boundings("ShowBoundingBoxes", "", false);

DECLARE_CPU_PROFILING_COUNTER(Counter_TotalCPU);
DECLARE_CPU_PROFILING_COUNTER(CpuCounter_RenderGraphCreate);

IMPLEMENT_CPU_PROFILING_COUNTER("Total CPU", "CPU", Counter_TotalCPU);
IMPLEMENT_CPU_PROFILING_COUNTER("RG Add", "RenderGraph", CpuCounter_RenderGraphCreate);

// TODO: move to appropriate place
static void DrawGameViewportWindow(Texture2* FinalTexture, const Matrix& ViewMatrix, const Matrix& ProjectionMatrix, iVector2& InOutViewSize, bool& OutWindowHover, bool& OutViewportFocus, Vector2& OutRelativeMousePosition)
{
	OutWindowHover = false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("Viewport"))
	{
		OutWindowHover = ImGui::IsWindowHovered();
		OutViewportFocus = ImGui::IsWindowFocused();

		ImVec2 MousePos = ImGui::GetMousePos();
		ImVec2 WindowPos = ImGui::GetWindowPos();
		ImVec2 viewportSize = ImGui::GetWindowSize();
		ImVec2 Cursor = ImGui::GetCursorPos();
		viewportSize.y -= Cursor.y;

		OutRelativeMousePosition = iVector2((int)(MousePos.x - WindowPos.x), (int)(MousePos.y - WindowPos.y - Cursor.y));

		bool InvalidateViewport = InOutViewSize.X != (int)viewportSize.x || InOutViewSize.Y != (int)viewportSize.y;

		InOutViewSize = iVector2((int)viewportSize.x, (int)viewportSize.y);

		DebugUI::TextureWidget(FinalTexture, InOutViewSize, InvalidateViewport);

		static Matrix TestMatrix;
		ImGuizmo::SetRect(WindowPos.x, WindowPos.y, viewportSize.x, viewportSize.y);
		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate(&ViewMatrix.M[0][0], &ProjectionMatrix.M[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, &TestMatrix.M[0][0]);
	}
	ImGui::End();
	ImGui::PopStyleVar(1);
}

// Engine structure that I find appropriate
// 1. Engine is a library, editor, apps, games are made using it
// 2. It uses it's own static code-generated reflection system
// 3. Usage of it to setup an app is simple:
//
// IntiailizeEngine();
// Window window = EngineCreateWindow();
// RenderDevice device = EngineCreateDevice();
// RenderGraph render = device.CreateRenderGraph();
// render.AddRenderPass(...);
//
// TODO: think about extensions (or engine modules for different systems), non-game and non-editor scenarios
// Scene scene = LoadScene(...); // uses ECS
// Object object = scene.AddObject(...);
// scene.AddComponent(object, ...);
//
// TODO: think about it
// !!!! Integrate TaskFlow
// TaskSystem taskSystem = EngineCreateTaskSystem();
//
// TODO: main loop, subsystems
// while (window.IsOpen())
// {
//      EngineUpdate();
//      scene.Update();
//      render.Render();
//      window.Present();
// }

// System tasks:
// 1. RenderGraph
//		+ very basic resource synchronization
//		+ RenderPass depth attachments
//		+ resource tracker and synchronization mechanism
//		+ renderpass blending
//		+ global/frame-local resources
//		+ swapchain resize (and pipeline dynamic parameters)
//		- resource aliasing
//		+ resource visualization
//		+ diagnostic information
//		- resource type validation
//		+ RW passes
//		+ storage/sampled images transitions
// 2. UI system (basic)
// 3. SceneGraph (and GPUScene)
//		+ GPUScene
//		- SceneGraph representation
//			- Static meshes
//			- Materials
//			- Lights
// 4. Static reflection
// 5. ECS
// 6. TaskGraph
// 7. CVar system (+console)
// 8. Config system
//
// Rendering tasks:
// 		1. Path Tracer (reference)
//			+ automatic buffer reset on camera move
//			+ Bounces setting
//			- BRDF and simple material declaration
//			- lightweight shadow rays
//			- lights:
//				+ point
//				+ directional
//				- spot
//				- rect
//				- sphere
//				- capsule
//			- importance sampling
//			- refractions
//			- volumetrics and OpenVDB
//			- spectral refraction
//			- adaptive sampling
//		2. Real Time
//			+ basic forward rendering
//			+ GBuffer pass
//			- ray-traced shadows with variable penumbra (TODO: denoiser)
//			+ directional lighting pass (lights + shadows)
//			- ray-traced spherical harmonics/ambient cubes (DDGI-ish solution)
//			+ baked path-traced lightmaps
//			- SSAO
//			- RTAO
//			- clustered rendering, GPU culling, vis buffer
//			- volumetrics and OpenVDB
//			- ray-traced translucency
//          - DDGI
//			- RT reflections (TODO: denoiser)
//			- GI1.0
//			- simple billboard particles render
//			+ upscaling (FSR1)
//          - TAA
//          - FSR2
//		3. Common
//			- PBR atmosphere rendering
//			- filmic camera
//			- filmic HDR tonemapper
//			- render image export from every stage of a graph (!!!)
//			+ decals
//			- subsurface scattering
//			- IES light profiles
//			+ shader include files
//			+ fix descriptor set duplication
//			- HLSL2021 instead of GLSL
//			+ make DebugRender work
//			- shader caching system
//
// Another high-level view of rendering system
// There should be a convinient way to define CPU-GPU logic for pipelines
// For example, define a shader that is compiled on an engine start (or by another policy)
// with defined data layout and a convinient way to modify data on per-pass basis
//
// GPUScene consists of GPU buffers with all geometry, textures, materials + TLAS
//
// Material system: ideally I want to support layered materials rendering, aka Substrate
//
// Rendering: RenderGraph-based, 2 default overlapping systems - Pathtraced and Realtime, + ability to extend it
// PathTracing - reference renderer, not realtime, but the most accurate one
// Realtime - rasterization based renderer, with realtime GI solution (DDGI or surfel-based)
//
// Think about multiple windows/viewports
// Think about multi-gpu
//
// POSSIBLY
// GPUParameters struct should contain descriptor sets
// So, Context.GetParameters<MyParameters>() will return a valid struct with a several preallocated descriptor sets
// Context.UpdateParameters<MyParameters>(params) will update descriptor sets based on data passed into it
// and then Context.BindParameters<MyParameters>(params)
//
// Therefore, GPUScene can be abstracted away with this (so, it defines several descriptor sets and how to map data from CPU to GPU)
// descriptor set layouts are allocated and recycled based on SPIR-V reflection
// How to generate descriptor set layouts for GPUScene when there are no shaders loaded?
//	- what data do we really need for descriptor set layout? can it be acquired from GPUParameters?
//	- solution 1 - define everything (indices, names, types) in GPUParameters, create descriptor set layouts using caching scheme
//	- solution 2 - simple - just create it in rendergraph
int main()
{
	InitializeEngine();

	Camera camera;
	Timer timer;

	camera.Update();

	char* SceneLoadPath = NULL;
	if (NFD_OpenDialog("gltf", NULL, &SceneLoadPath) != NFD_OKAY)
	{
		return 1;
	}

	Columbus::InstanceVulkan instance;
	auto device = instance.CreateDevice();

	EngineWorld World;
	World.Device = device;
	World.LoadLevelGLTF(SceneLoadPath);

	auto renderGraph = RenderGraph(device, World.SceneGPU);
	WindowVulkan Window(instance, device);
	DeferredRenderContext DeferredContext;

	World.MainView.OutputSize = Window.GetSize();

	bool bViewportHover = false;
	bool bViewportFocused = false;
	Vector2 ViewportMousePos;
	Vector2 ViewportSize;

	DebugUI::Context* UiContext = DebugUI::Create(&Window);

	bool running = true;
	while (running)
	{
		device->BeginFrame();
		World.BeginFrame();
		DebugUI::BeginFrame(UiContext);
		PROFILE_CPU(Counter_TotalCPU);

		float DeltaTime = timer.Elapsed();
		timer.Reset();

		// TODO:
		camera.Perspective(45, (float)World.MainView.OutputSize.X / (float)World.MainView.OutputSize.Y, 0.1f, 5000.f);
		camera.Update();
		World.MainView.CameraCur = camera;

		// Input
		{
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				DebugUI::ProcessInputSDL(UiContext, &event);

				if (event.type == SDL_QUIT) {
					running = false;
				}

				if (event.type == SDL_WINDOWEVENT) {
					if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_MAXIMIZED) {
						device->QueueWaitIdle();
						Window.Swapchain->IsOutdated = true;
						Window.OnResize(iVector2{ event.window.data1, event.window.data2 });

						Log::Message("Window resized: %i %i", event.window.data1, event.window.data2);
					}
				}
			}
		}

		// UI
		{
			DebugUI::DrawMainLayout();

			ImGui::ShowDemoWindow();
			ShowDebugConsole();
			ShowRenderGraphVisualiser(renderGraph);

			DebugUI::ShowScreenshotSaveWindow(World.MainView);

			DebugUI::ShowMeshesWindow(World);
			DebugUI::ShowDecalsWindow(World);
			DebugUI::ShowLightsWindow(World);
			DebugUI::ShowMaterialsWindow(World);
			DebugUI::ShowIrradianceWindow(World);
			DebugUI::ShowLightmapWindow(World);

			// test gaussian
			{
				static float Peak = 1;
				static float Offset = 0.5f;
				static float StdDev = 1;

				const auto Gaussian = [](float x)
				{
					return Peak * expf(-powf(x-Offset, 2) / 2*StdDev*StdDev);
				};

				if (ImGui::Begin("Gaussian Test"))
				{
					ImGui::SliderFloat("Peak", &Peak, 0, 5);
					ImGui::SliderFloat("Offset", &Offset, -0.5f, 0.5f);
					ImGui::SliderFloat("StdDev", &StdDev, 0, 5);

					float x[1000];
					float y[1000];

					for (int i = 0; i < 1000; i++)
					{
						x[i] = i / (float)1000;
						y[i] = Gaussian(x[i]);
					}

					if (ImPlot::BeginPlot("Gaussian"))
					{
						ImPlot::PlotLine("Graph", x, y, 1000);
						ImPlot::EndPlot();
					}
				}
				ImGui::End();
			}
		}

		World.UpdateTransforms();

		// mouse picking
		if (bViewportHover && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)))
		{
			ImGuiIO& io = ImGui::GetIO();
			Vector2 MousePos = ViewportMousePos;
			Vector2 DisplaySize = ViewportSize;
			Vector2 MouseNormalised = MousePos / DisplaySize;

			WorldIntersectionResult Intersection = World.CastCameraRayClosestHit(MouseNormalised);
			if (Intersection.HasIntersection)
			{
				Geometry::Triangle Tri = Intersection.Triangle;
				Vector3 triOffset = Tri.Normal() * 0.01f;
				World.MainView.DebugRender.AddTri(Tri.A + triOffset, Tri.B + triOffset, Tri.C + triOffset, { 1, 0, 0, 1 });

				CPUSceneMesh& Mesh = World.SceneCPU.Meshes[Intersection.MeshId];

				u32 Index1 = Mesh.Indices[Intersection.TriangleId * 3 + 0];
				u32 Index2 = Mesh.Indices[Intersection.TriangleId * 3 + 1];
				u32 Index3 = Mesh.Indices[Intersection.TriangleId * 3 + 2];

				Vector3 Normal1 = Mesh.Normals[Index1];
				Vector3 Normal2 = Mesh.Normals[Index2];
				Vector3 Normal3 = Mesh.Normals[Index3];

				float Length = 30;
				float LineWidth = 2;

				Vector3 BasisOffset = Tri.Normal() * LineWidth / 2;

				// normals
				World.MainView.DebugRender.AddLineFromTo(Tri.A + BasisOffset, Tri.A + BasisOffset + Normal1 * Length, LineWidth, Vector4(0, 0, 1, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.B + BasisOffset, Tri.B + BasisOffset + Normal2 * Length, LineWidth, Vector4(0, 0, 1, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.C + BasisOffset, Tri.C + BasisOffset + Normal3 * Length, LineWidth, Vector4(0, 0, 1, 1));

				Vector3 Tangent1 = Mesh.Tangents[Index1].XYZ();
				Vector3 Tangent2 = Mesh.Tangents[Index2].XYZ();
				Vector3 Tangent3 = Mesh.Tangents[Index3].XYZ();

				Vector3 Bitangent1 = Vector3::Cross(Tangent1, Normal1).Normalized() * Mesh.Tangents[Index1].W;
				Vector3 Bitangent2 = Vector3::Cross(Tangent2, Normal2).Normalized() * Mesh.Tangents[Index2].W;
				Vector3 Bitangent3 = Vector3::Cross(Tangent3, Normal3).Normalized() * Mesh.Tangents[Index3].W;

				// tangents
				World.MainView.DebugRender.AddLineFromTo(Tri.A + BasisOffset, Tri.A + BasisOffset + Tangent1 * Length, LineWidth, Vector4(1, 0, 0, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.B + BasisOffset, Tri.B + BasisOffset + Tangent2 * Length, LineWidth, Vector4(1, 0, 0, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.C + BasisOffset, Tri.C + BasisOffset + Tangent3 * Length, LineWidth, Vector4(1, 0, 0, 1));

				// bitangents
				World.MainView.DebugRender.AddLineFromTo(Tri.A + BasisOffset, Tri.A + BasisOffset + Bitangent1 * Length, LineWidth, Vector4(0, 1, 0, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.B + BasisOffset, Tri.B + BasisOffset + Bitangent2 * Length, LineWidth, Vector4(0, 1, 0, 1));
				World.MainView.DebugRender.AddLineFromTo(Tri.C + BasisOffset, Tri.C + BasisOffset + Bitangent3 * Length, LineWidth, Vector4(0, 1, 0, 1));
			}
		}

		if (CVar_Boundings.GetValue())
		{
			for (int i = 0; i < (int)World.SceneGPU->Meshes.size(); i++)
			{
				const GPUSceneMesh& Mesh = World.SceneGPU->Meshes[i];
				const Box& Bounding = World.MeshBoundingBoxes[i];
				const Vector3 Center = Bounding.CalcCenter();
				const Vector3 Extent = Bounding.CalcSize();

				Matrix Transform;
				Transform.Scale(Extent);
				Transform.Translate(Center);
				Transform = Mesh.Transform * Transform;

				World.MainView.DebugRender.AddBox(Transform, Vector4(0, 0.3f, 0, 0.3f));
			}
		}

		// rendergraph stuff
		{
			RenderGraphTextureRef FinalTexture = -1;
			renderGraph.Clear();

			// setup render passes
			{
				PROFILE_CPU(CpuCounter_RenderGraphCreate);

				UploadGPUSceneRG(renderGraph);
				// TODO: rebuild/refit TLAS

				if (render_cvar.GetValue() == 0)
				{
					if (World.Lightmaps.BakingRequested)
					{
						PrepareAtlasForLightmapBaking(renderGraph, World.Lightmaps);
						BakeLightmapPathTraced(renderGraph, World.Lightmaps);
					}

#if 0
					if (scene->IrradianceVolumes[0].ProbesBuffer == nullptr || ComputeIrradianceVolume)
					{
						RenderIrradianceProbes(renderGraph, World.MainView, scene->IrradianceVolumes[0]);
						ComputeIrradianceVolume = false;
					}
#endif

					FinalTexture = RenderDeferred(renderGraph, World.MainView, DeferredContext);
				}
				else
				{
					FinalTexture = RenderPathTraced(renderGraph, World.MainView);
				}
			}

			// RG execution/present
			{
				WindowSwapchainAcquireData AcquireData = Window.AcquireNextSwapchainImage();
				Texture2* SwapchainTexture = AcquireData.Image;

				VkSemaphore WaitSemaphore = AcquireData.ImageAcquiredSemaphore;

				// main render graph and viewport widget
				{
					RenderGraphExecuteParameters RGParameters;
					RGParameters.DefaultViewportSize = Window.GetSize();
					RGParameters.WaitSemaphore = WaitSemaphore;
					RenderGraphExecuteResults RGResults = renderGraph.Execute(RGParameters);
					WaitSemaphore = RGResults.FinishSemaphore;

					DrawGameViewportWindow(renderGraph.GetTextureAfterExecution(FinalTexture), camera.GetViewMatrix(), camera.GetProjectionMatrix(), World.MainView.OutputSize, bViewportHover, bViewportFocused, ViewportMousePos);
					ViewportSize = World.MainView.OutputSize;
				}

				// UI rendering
				{
					DebugUI::RenderResult Result = DebugUI::Render(UiContext, WaitSemaphore);
					WaitSemaphore = Result.FinishSemaphore;

					Window.Present(WaitSemaphore, Result.ResultTexture);
				}
			}
		}

		// std::string graphviz = renderGraph.ExportGraphviz();
		// printf("%s\n", graphviz.c_str());
		// return 0;

		// TODO: viewport movement controlling system
		if (bViewportFocused)
		{
			auto keyboard = SDL_GetKeyboardState(NULL);
			if (keyboard[SDL_SCANCODE_DOWN]) camera.Rot += Columbus::Vector3(5,0,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_UP]) camera.Rot += Columbus::Vector3(-5,0,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_LEFT]) camera.Rot += Columbus::Vector3(0,5,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_RIGHT]) camera.Rot += Columbus::Vector3(0,-5,0) * DeltaTime * 20;

			float CameraSpeed = CVar_CameraSpeed.GetValue();
			if (keyboard[SDL_SCANCODE_W]) camera.Pos += camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_S]) camera.Pos -= camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_D]) camera.Pos += camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_A]) camera.Pos -= camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LSHIFT]) camera.Pos += camera.Up() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LCTRL]) camera.Pos -= camera.Up() * DeltaTime * CameraSpeed;
		}

		DebugUI::EndFrame(UiContext);
		World.EndFrame();
		device->EndFrame();
	}

	DebugUI::Destroy(UiContext);

	// TODO: proper cleanup
	VK_CHECK(vkQueueWaitIdle(*device->_ComputeQueue));
	VK_CHECK(vkDeviceWaitIdle(device->_Device));

	//SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
