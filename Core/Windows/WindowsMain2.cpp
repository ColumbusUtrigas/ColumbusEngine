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

// TEST
#include <Physics/PhysicsWorld.h>
#include <Physics/PhysicsShapeConvexHull.h>
#include <Physics/PhysicsShapeSphere.h>

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

// TODO: make it normal
int SelectedObject = -1;

struct Player
{
	EngineWorld& World;

	int MeshId;
	GameObjectId Id;
	Rigidbody* RB;

	Player(EngineWorld& World) : World(World), Id(-1) {}

	void Load();
	void Spawn();

	void HandleInput();

	void PrePhysics();
	void PostPhysics();
};

void Player::Load()
{
	Model model;
	model.Load("Data/Meshes/Sphere.obj");
	//model.Load("Data/Meshes/Hercules.cmf");
	model.RecalculateTangents();
	MeshId = World.LoadMesh(model);

	SubModel a = model.GetSubModel(0);

	Vector3 Center;
	float Radius = 0;
	{
		for (int i = 0; i < a.VerticesCount; i++)
		{
			Center += a.Positions[i];
		}

		Center /= a.VerticesCount;

		for (int i = 0; i < a.VerticesCount; i++)
		{
			Radius = Math::Max(a.Positions[i].Distance(Center), Radius);
		}
	}

	PhysicsShapeConvexHull* Hull = new PhysicsShapeConvexHull((float*)a.Positions, a.VerticesCount);
	PhysicsShapeSphere* Sphere = new PhysicsShapeSphere(Radius);

	RB = new Rigidbody(Sphere);
	RB->SetStatic(false);
}

void Player::Spawn()
{
	if (Id != -1)
		return; // already spawned

	Id = World.CreateGameObject("Player", MeshId);
	World.GameObjects[Id].Trans.Position = Vector3(0, 100, 0);
	World.GameObjects[Id].Trans.Update();

	RB->SetTransform(World.GameObjects[Id].Trans);

	World.Physics.AddRigidbody(RB);
}

void Player::HandleInput()
{
	auto keyboard = SDL_GetKeyboardState(NULL);
	if (keyboard[SDL_SCANCODE_I]) RB->ApplyCentralImpulse(Vector3(0, 0, +1));
	if (keyboard[SDL_SCANCODE_J]) RB->ApplyCentralImpulse(Vector3(+1, 0, 0));
	if (keyboard[SDL_SCANCODE_K]) RB->ApplyCentralImpulse(Vector3(0, 0, -1));
	if (keyboard[SDL_SCANCODE_L]) RB->ApplyCentralImpulse(Vector3(-1, 0, 0));
}

void Player::PrePhysics()
{
	if (Id != -1)
	{
		RB->SetTransform(World.GameObjects[Id].Trans);
	}
}

void Player::PostPhysics()
{
	if (Id != -1)
	{
		World.GameObjects[Id].Trans = RB->GetTransform();
		World.GameObjects[Id].Trans.Update();
	}
}

// TODO: move to appropriate place
static void DrawGameViewportWindow(Texture2* FinalTexture, EngineWorld& World, Matrix ViewMatrix, Matrix ProjectionMatrix, iVector2& InOutViewSize, bool& OutWindowHover, bool& OutViewportFocus, Vector2& OutRelativeMousePosition)
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

		// to column-major for ImGuizmo
		ViewMatrix.Transpose();
		ProjectionMatrix.Transpose();

		if (SelectedObject != -1)
		{
			// TODO: proper scene graph and transform management

			Transform& Trans = World.GameObjects[SelectedObject].Trans;

			Matrix TestMatrix = Trans.GetMatrix().GetTransposed();
			ImGuizmo::SetRect(WindowPos.x, WindowPos.y, viewportSize.x, viewportSize.y);
			ImGuizmo::SetDrawlist();
			ImGuizmo::Manipulate(&ViewMatrix.M[0][0], &ProjectionMatrix.M[0][0], ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, &TestMatrix.M[0][0]);

			Vector3 Euler;
			TestMatrix.Transpose();
			TestMatrix.DecomposeTransform(Trans.Position, Euler, Trans.Scale);
			Trans.Rotation = Quaternion(Euler);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(1);
}

static void AddWorldCollision(EngineWorld& World)
{
	// create rigidbodies for the level
	for (int i = 0; i < (int)World.GameObjects.size(); i++)
	{
		const Mesh2& mesh = *World.Meshes[World.GameObjects[i].MeshId];

		for (int j = 0; j < (int)mesh.Primitives.size(); j++)
		{
			int numFaces = mesh.Primitives[j].CPU.Indices.size() / 3;
			int vertStride = sizeof(Vector3);
			int indexStride = 3 * sizeof(u32);
			btTriangleIndexVertexArray* va = new btTriangleIndexVertexArray(numFaces,
				(int*)mesh.Primitives[j].CPU.Indices.data(),
				indexStride,
				mesh.Primitives[j].CPU.Vertices.size(), (btScalar*)mesh.Primitives[j].CPU.Vertices.data(), vertStride);
			btBvhTriangleMeshShape* triShape = new btBvhTriangleMeshShape(va, true);

			Rigidbody* MeshRB = new Rigidbody(triShape);
			MeshRB->SetStatic(true);
			World.Physics.AddRigidbody(MeshRB);
		}
	}
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
//		+ SceneGraph representation
//			+ Static meshes
//			+ Materials
//			+ Lights
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
//			+ BRDF and simple material declaration
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
//          + RTGI (first bounce, diffuse)
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
//          - material layers
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
	// World.SceneGPU = SPtr<GPUScene>(GPUScene::CreateGPUScene(device)); // empty level

	AddWorldCollision(World);

	Player player(World);
	player.Load();

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

		float DeltaTime = (float)timer.Elapsed();
		timer.Reset();

		// TODO:
		camera.Perspective(45, (float)World.MainView.OutputSize.X / (float)World.MainView.OutputSize.Y, 0.1f, 5000.f);
		camera.Update();
		World.MainView.CameraCur = camera;

		iVector2 MouseMotion;

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

				if (event.type == SDL_MOUSEMOTION) {
					MouseMotion.X += event.motion.xrel;
					MouseMotion.Y += event.motion.yrel;
				}

				if (event.type == SDL_MOUSEWHEEL) {
					float CameraSpeed = CVar_CameraSpeed.GetValue();
					CameraSpeed += float(event.wheel.y);
					CVar_CameraSpeed.SetValue(CameraSpeed);
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

			if (ImGui::Begin("Camera"))
			{
				ImGui::InputFloat3("Camera Position", (float*)&camera.Pos);
				ImGui::InputFloat3("Camera Rotation", (float*)&camera.Rot);
			}
			ImGui::End();

			// test gaussian
			{
				static float Peak = 1;
				static float Offset = 0.5f;
				static float StdDev = 1;
				static float XMin = -4;
				static float XMax = 4;

				const auto Gaussian = [](float x)
				{
					return Peak * expf(-powf(x-Offset, 2) / 2*StdDev*StdDev);
				};

				if (ImGui::Begin("Gaussian Test"))
				{
					ImGui::SliderFloat("Peak", &Peak, 0, 5);
					ImGui::SliderFloat("Offset", &Offset, -0.5f, 0.5f);
					ImGui::SliderFloat("StdDev", &StdDev, 0, 5);
					ImGui::InputFloat("X Min", &XMin);
					ImGui::InputFloat("X Max", &XMax);

					float x[1000];
					float y[1000];

					for (int i = 0; i < 1000; i++)
					{
						x[i] = i / (float)1000 * (XMax - XMin) + XMin;
						y[i] = Gaussian(x[i]);
					}

					// find area under the curve
					float xDist = x[1] - x[0]; // distance between samples
					float area = 0;
					for (int i = 0; i < 1000; i++)
					{
						area += xDist * y[i];
					}

					ImGui::Text("Area: %f", area);

					if (ImPlot::BeginPlot("Gaussian"))
					{
						ImPlot::PlotLine("Graph", x, y, 1000);
						ImPlot::EndPlot();
					}
				}
				ImGui::End();
			}
		}

		player.PrePhysics();
		World.Update(DeltaTime);
		player.PostPhysics();

		// mouse picking
		if (bViewportHover && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) && !ImGuizmo::IsOver())
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

				CPUMeshResource& Mesh = World.Meshes[Intersection.MeshId]->Primitives[Intersection.MeshPrimitiveId].CPU;

				u32 Index1 = Mesh.Indices[Intersection.TriangleId * 3 + 0];
				u32 Index2 = Mesh.Indices[Intersection.TriangleId * 3 + 1];
				u32 Index3 = Mesh.Indices[Intersection.TriangleId * 3 + 2];

				Vector3 Normal1 = Mesh.Normals[Index1];
				Vector3 Normal2 = Mesh.Normals[Index2];
				Vector3 Normal3 = Mesh.Normals[Index3];

				float Length = Math::Min(Math::Min(Tri.A.Distance(Tri.B), Tri.A.Distance(Tri.C)), Tri.B.Distance(Tri.C));
				float LineWidth = Length / 10.f;

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

				SelectedObject = Intersection.ObjectId;
			}
		}

		if (CVar_Boundings.GetValue())
		{
			for (int i = 0; i < (int)World.SceneGPU->Meshes.size(); i++)
			{
				const GPUSceneMesh& Mesh = World.SceneGPU->Meshes[i];
				const Box& Bounding = World.Meshes[i]->BoundingBox;
				const Vector3 Center = Bounding.CalcCenter();
				const Vector3 Extent = Bounding.CalcSize();

				Matrix Transform;
				Transform.Scale(Extent);
				Transform.Translate(Center);
				Transform = Mesh.Transform * Transform;

				World.MainView.DebugRender.AddBox(Transform, Vector4(0, 0.3f, 0, 0.3f));
			}
		}

		// TODO: common object selection interface
		if (SelectedObject != -1)
		{
			const int MeshId = World.GameObjects[SelectedObject].MeshId;
			const Box Bounding = World.Meshes[MeshId]->BoundingBox;
			Matrix Transform = World.GameObjects[SelectedObject].Trans.GetMatrix();

			Transform.Scale(Bounding.CalcSize());
			Transform.Translate(Bounding.CalcCenter());

			World.MainView.DebugRender.AddBox(Transform, Vector4(0, 0.3f, 0, 0.3f));
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

					DrawGameViewportWindow(renderGraph.GetTextureAfterExecution(FinalTexture), World, camera.GetViewMatrix(), camera.GetProjectionMatrix(), World.MainView.OutputSize, bViewportHover, bViewportFocused, ViewportMousePos);
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

			if (keyboard[SDL_SCANCODE_ESCAPE]) SelectedObject = -1;

			float CameraSpeed = CVar_CameraSpeed.GetValue();
			if (keyboard[SDL_SCANCODE_W]) camera.Pos += camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_S]) camera.Pos -= camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_D]) camera.Pos += camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_A]) camera.Pos -= camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LSHIFT]) camera.Pos += camera.Up() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LCTRL]) camera.Pos -= camera.Up() * DeltaTime * CameraSpeed;

			player.HandleInput();

			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				camera.Rot += Vector3(MouseMotion.Y / 10.0f, -MouseMotion.X / 10.0f, 0);
			}

			if (keyboard[SDL_SCANCODE_P])
			{
				player.Spawn();
			}
		}

		DebugUI::EndFrame(UiContext);
		World.EndFrame();
		device->EndFrame();
	}

	DebugUI::Destroy(UiContext);

	// TODO: proper cleanup
	VK_CHECK(vkQueueWaitIdle(*device->_ComputeQueue));
	VK_CHECK(vkDeviceWaitIdle(device->_Device));

	DeferredContext.History.Destroy(device);

	//SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
