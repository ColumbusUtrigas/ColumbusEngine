#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Editor/CommonUI.h"
#include "Graphics/Camera.h"
#include "Graphics/GPUScene.h"
#include "Graphics/IrradianceVolume.h"
#include "Graphics/Vulkan/CommandBufferVulkan.h"
#include "Graphics/Vulkan/TextureVulkan.h"
#include "Input/Events.h"
#include "Input/Input.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/Quaternion.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "SDL_events.h"
#include "SDL_video.h"
#include "Scene/Transform.h"
#include "Lib/imgui/imgui.h"
#include "Lib/imgui/backends/imgui_impl_vulkan.h"
#include "Lib/imgui/backends/imgui_impl_sdl2.h"
#include "System/File.h"
#include <cstdint>
#include <memory>
#include <thread>
#include <chrono>
#include <vulkan/vulkan.h>
#include <Core/Core.h>
#include <Core/CVar.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/RenderPasses/RenderPasses.h>
#include <Graphics/Lightmaps.h>
#include <Profiling/Profiling.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <Graphics/Vulkan/InstanceVulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>

#include <vector>
#include <algorithm>
#include <queue>

#include <Lib/tinygltf/tiny_gltf.h>

using namespace Columbus;

class WindowVulkan
{
public:
	WindowVulkan(InstanceVulkan& Instance, SPtr<DeviceVulkan> Device) : Instance(Instance), Device(Device)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		Window = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Size.X, Size.Y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
		SDL_Vulkan_CreateSurface(Window, Instance.instance, &Surface);

		Swapchain = Device->CreateSwapchain(Surface, nullptr);
	}

	void OnResize(iVector2 NewSize)
	{
		if (Size != NewSize)
		{
			RecreateSwapchain();
		}
	}

	void RecreateSwapchain()
	{
		SwapchainVulkan* newSwapchain = Device->CreateSwapchain(Surface, Swapchain);
		delete Swapchain;
		Swapchain = newSwapchain;

		// Size = NewSize;
		Size = iVector2((int)Swapchain->swapChainExtent.width, (int)newSwapchain->swapChainExtent.height);
	}

	~WindowVulkan()
	{
		delete Swapchain;
		vkDestroySurfaceKHR(Instance.instance, Surface, nullptr);
		SDL_DestroyWindow(Window);
	}

public:
	SDL_Window* Window;
	VkSurfaceKHR Surface;
	SwapchainVulkan* Swapchain;

	InstanceVulkan& Instance;
	SPtr<DeviceVulkan> Device;

	iVector2 Size{1280, 720};
};


DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTextures);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneMeshes);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneBLAS);
DECLARE_MEMORY_PROFILING_COUNTER(MemoryCounter_SceneTLAS);

IMPLEMENT_MEMORY_PROFILING_COUNTER("Textures", "SceneMemory", MemoryCounter_SceneTextures);
IMPLEMENT_MEMORY_PROFILING_COUNTER("Meshes", "SceneMemory", MemoryCounter_SceneMeshes);
IMPLEMENT_MEMORY_PROFILING_COUNTER("BLAS", "SceneMemory", MemoryCounter_SceneBLAS);
IMPLEMENT_MEMORY_PROFILING_COUNTER("TLAS", "SceneMemory", MemoryCounter_SceneTLAS);

// TODO: move to appropriate place
Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data)
{
	BufferDesc desc;
	desc.BindFlags = BufferType::UAV;
	desc.Size = size;
	desc.UsedInAccelerationStructure = usedInAS;
	Buffer* result = device->CreateBuffer(desc, data);
	AddProfilingMemory(MemoryCounter_SceneMeshes, result->GetSize());
	return result;
}

// TODO: move to appropriate place
SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename, CPUScene& cpuScene)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	if (!loader.LoadASCIIFromFile(&model, &err, &warn, Filename))
	{
		Log::Fatal("Couldn't load scene, %s", Filename.c_str());
	}

	SPtr<GPUScene> Scene = SPtr<GPUScene>(GPUScene::CreateGPUScene(Device), [Device](GPUScene* Scene)
	{
		for (auto& Texture : Scene->Textures)
		{
			Device->DestroyTexture(Texture);
		}

		for (auto& Mesh : Scene->Meshes)
		{
			Device->DestroyBuffer(Mesh.Vertices);
			Device->DestroyBuffer(Mesh.Indices);
			Device->DestroyBuffer(Mesh.UV1);
			if (Mesh.UV2)
				Device->DestroyBuffer(Mesh.UV2);
			Device->DestroyBuffer(Mesh.Normals);
			Device->DestroyAccelerationStructure(Mesh.BLAS);
		}

		for (auto& Decal : Scene->Decals)
		{
			Device->DestroyTexture(Decal.Texture);
		}

		Device->DestroyAccelerationStructure(Scene->TLAS);

		GPUScene::DestroyGPUScene(Scene, Device);
	});

	for (auto& texture : model.textures)
	{
		auto& image = model.images[texture.source];

		Image img;
		img.FromMemory(image.image.data(), image.image.size(), image.width, image.height);

		auto tex = Device->CreateTexture(img);
		Device->SetDebugName(tex, texture.name.c_str());
		AddProfilingMemory(MemoryCounter_SceneTextures, tex->GetSize());
		Scene->Textures.push_back(tex);
	}

	// TODO
	for (auto& mesh : model.meshes)
	{
		for (auto& primitive : mesh.primitives)
		{
			Buffer* indexBuffer = nullptr;
			Buffer* vertexBuffer = nullptr;
			Buffer* uvBuffer = nullptr;
			Buffer* normalBuffer = nullptr;
			Buffer* materialBuffer = nullptr;

			cpuScene.Meshes.push_back(CPUSceneMesh{});
			CPUSceneMesh& cpuMesh = cpuScene.Meshes.back();

			int indicesCount = 0;
			int verticesCount = 0;

			{
				auto accessor = model.accessors[primitive.indices];
				auto view = model.bufferViews[accessor.bufferView];
				auto buffer = model.buffers[view.buffer];
				auto offset = accessor.byteOffset + view.byteOffset;
				const void* data = buffer.data.data() + offset;
				std::vector<uint32_t> indices(accessor.count);
				indicesCount = accessor.count;

				switch (accessor.componentType)
				{
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						for (int i = 0; i < accessor.count; i++)
						{
							indices[i] = static_cast<const uint16_t*>(data)[i];
						}
						break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					for (int i = 0; i < accessor.count; i++)
						{
							indices[i] = static_cast<const uint32_t*>(data)[i];
						}
						break;
					default: COLUMBUS_ASSERT(false);
				}

				cpuMesh.Indices = indices;

				indexBuffer = CreateMeshBuffer(Device, indices.size() * sizeof(uint32_t), true, indices.data());
				Device->SetDebugName(indexBuffer, (mesh.name + " (Indices)").c_str());
			}

			{
				auto accessor = model.accessors[primitive.attributes["POSITION"]];
				auto view = model.bufferViews[accessor.bufferView];
				auto buffer = model.buffers[view.buffer];
				auto offset = accessor.byteOffset + view.byteOffset;
				const void* data = buffer.data.data() + offset;
				verticesCount = accessor.count;

				cpuMesh.Vertices = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data) + verticesCount);

				vertexBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
				Device->SetDebugName(vertexBuffer, (mesh.name + " (Vertices)").c_str());
			}

			{
				auto accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
				auto view = model.bufferViews[accessor.bufferView];
				auto buffer = model.buffers[view.buffer];
				auto offset = accessor.byteOffset + view.byteOffset;
				const void* data = buffer.data.data() + offset;
				verticesCount = accessor.count;

				cpuMesh.UV1 = std::vector<Vector2>((Vector2*)(data), (Vector2*)(data) + verticesCount);

				uvBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector2), true, data);
				Device->SetDebugName(uvBuffer, (mesh.name + " (UVs)").c_str());
			}

			{
				auto accessor = model.accessors[primitive.attributes["NORMAL"]];
				auto view = model.bufferViews[accessor.bufferView];
				auto buffer = model.buffers[view.buffer];
				auto offset = accessor.byteOffset + view.byteOffset;
				const void* data = buffer.data.data() + offset;
				verticesCount = accessor.count;

				cpuMesh.Normals = std::vector<Vector3>((Vector3*)(data), (Vector3*)(data) + verticesCount);

				normalBuffer = CreateMeshBuffer(Device, accessor.count * sizeof(Vector3), true, data);
				Device->SetDebugName(normalBuffer, (mesh.name + " (Normals)").c_str());
			}

			Columbus::AccelerationStructureDesc blasDesc;
			blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
			blasDesc.Vertices = vertexBuffer;
			blasDesc.Indices = indexBuffer;
			blasDesc.VerticesCount = verticesCount;
			blasDesc.IndicesCount = indicesCount;
			auto BLAS = Device->CreateAccelerationStructure(blasDesc);

			if (Device->SupportsRayTracing())
			{
				Device->SetDebugName(BLAS, mesh.name.c_str());
				AddProfilingMemory(MemoryCounter_SceneBLAS, BLAS->GetSize());
			}

			int matid = -1;

			if (primitive.material > -1)
			{
				auto mat = model.materials[primitive.material];
				matid = mat.pbrMetallicRoughness.baseColorTexture.index;
			}

			GPUSceneMesh Mesh;
			Mesh.BLAS = BLAS;
			Mesh.Transform = Matrix(1);
			Mesh.Vertices = vertexBuffer;
			Mesh.Indices = indexBuffer;
			Mesh.UV1 = uvBuffer;
			Mesh.Normals = normalBuffer;
			Mesh.VertexCount = verticesCount;
			Mesh.IndicesCount = indicesCount;
			Mesh.TextureId = matid;

			Scene->Meshes.push_back(Mesh);
		}
	}

	// TLAS and BLASes should be packed into GPU scene
	AccelerationStructureDesc TlasDesc;
	TlasDesc.Type = AccelerationStructureType::TLAS;
	TlasDesc.Instances = {};
	for (auto& mesh : Scene->Meshes)
	{
		TlasDesc.Instances.push_back({ Matrix(), mesh.BLAS });
	}

	Scene->TLAS = Device->CreateAccelerationStructure(TlasDesc);
	Scene->MainCamera = GPUCamera(DefaultCamera);

	if (Device->SupportsRayTracing())
	{
		AddProfilingMemory(MemoryCounter_SceneTLAS, Scene->TLAS->GetSize());
	}

	Scene->Lights.push_back(GPULight{{}, {1,1,1,0}, {1,1,1,0}, LightType::Directional, 0, 0.1f});
	Scene->Lights.push_back(GPULight{{0,200,0,0}, {}, {50,0,0,0}, LightType::Point, 500, 0});

	{
		Image DecalImage;
		if (!DecalImage.Load("./Data/Textures/Detail.dds"))
		{
			Log::Error("Couldn't load decal image");
		}

		Texture2* DecalTexture = Device->CreateTexture(DecalImage);

		Matrix Decal;
		Decal.Scale({100});
		Scene->Decals.push_back(GPUDecal{Decal, Decal.GetInverted(), DecalTexture});
	}

	return Scene;
}

ConsoleVariable<bool> test_flag("test.flag", "Description", true);
ConsoleVariable<int> render_cvar("r.Render", "0 - Deferred, 1 - PathTraced, default - 1", 0);

DECLARE_CPU_PROFILING_COUNTER(Counter_TotalCPU);
DECLARE_CPU_PROFILING_COUNTER(CpuCounter_RenderGraphCreate);

IMPLEMENT_CPU_PROFILING_COUNTER("Total CPU", "CPU", Counter_TotalCPU);
IMPLEMENT_CPU_PROFILING_COUNTER("RG Add", "RenderGraph", CpuCounter_RenderGraphCreate);

void InitializeImgui(WindowVulkan& Window, SPtr<DeviceVulkan> Device)
{
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForVulkan(Window.Window);
	SetupImguiForSwapchain(Device, Window.Swapchain);
}

void NewFrameImgui(WindowVulkan& Window)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Editor::ApplyDarkTheme();
	io.DisplaySize = {(float)Window.Size.X, (float)Window.Size.Y};

	if (io.Fonts->IsBuilt())
	{
		ImGui::NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(Window.Window);
	}
}

// TODO: think about it, is it REALLY needed? sounds like an overengineered system for nothing
// BEGIN_GPU_PARAMETERS(GPUSceneParameters)
//		GPU_PARAMETERS_UNBOUNDED_ARRAY(Textures, GPU_PARAMETER_TEXTURE2D, 0)
//		GPU_PARAMETERS_UNBOUNDED_ARRAY(Vertices, GPU_PARAMETER_BUFFER, 1)
//		GPU_PARAMETERS_UNBOUNDED_ARRAY(Materials, GPU_PARAMETER_BUFFER, 2)
// END_GPU_PARAMETERS()

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
//		- resource visualization
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
//			- DDGI+RT reflections
//			- GI1.0
//			- simple billboard particles render
//			- upscaling (simple and FSR)
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

	Camera camera, cameraPrev;
	Columbus::Timer timer;
	// camera.Pos = { 0, 300, 104 };
	camera.Rot = { 0, -70, 0 };
	camera.Perspective(45, 1280.f/720.f, 1.f, 5000.f);
	float CameraSpeed = 10;

	camera.Update();
	cameraPrev = camera;

	// SPtr<GPUScene> scene = std::make_shared<GPUScene>();

	Columbus::InstanceVulkan instance;
	auto device = instance.CreateDevice();

	CPUScene cpuScene;
	LightmapSystem lightmapSystem;

	// auto scene = LoadScene(device, camera, "D:/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
	// auto scene = LoadScene(device, camera, "/home/columbus/assets/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf");
	// auto scene = LoadScene(device, camera, "/home/columbus/assets/glTF-Sample-Models-master/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
	auto scene = LoadScene(device, camera, "/home/columbus/assets/cp.gltf", cpuScene);
	auto renderGraph = RenderGraph(device, scene);
	WindowVulkan Window(instance, device);
	DeferredRenderContext DeferredContext; // for deferred

	// TODO: DDGI, that's a CPU-side representation, needs to be updated during GPUScene update stage and then used in a rendergraph
	IrradianceVolume Volume;
	Volume.Position = { 0, 0, 0 };
	Volume.ProbesCount = { 5, 4, 5 };
	Volume.Extent = { 10, 5, 10 };
	scene->IrradianceVolumes.push_back(Volume);
	bool ComputeIrradianceVolume = false;

	DebugRender debugRender;

	InitializeImgui(Window, device);

	bool running = true;
	while (running)
	{
		ResetProfiling();
		device->BeginFrame();
		PROFILE_CPU(Counter_TotalCPU);

		float DeltaTime = timer.Elapsed();
		timer.Reset();

		camera.Perspective(45, (float)Window.Size.X / (float)Window.Size.Y, 1.f, 5000.f);
		camera.Update();
		debugRender.Clear();

		NewFrameImgui(Window);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT) {
				running = false;
			}

			if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					device->QueueWaitIdle();
					Window.Swapchain->IsOutdated = true;
					Window.OnResize(iVector2{event.window.data1, event.window.data2});

					Log::Message("Window resized: %i %i", event.window.data1, event.window.data2);
				}
			}
		}

		// UI
		{
			ImGui::ShowDemoWindow();
			ShowDebugConsole();
			ShowRenderGraphVisualiser(renderGraph);

			// lightmap
			{
				if (ImGui::Begin("Lightmap"))
				{
					ImGui::InputInt("Samples", &lightmapSystem.BakingSettings.RequestedSamples);
					ImGui::InputInt("Bounces", &lightmapSystem.BakingSettings.Bounces);
					ImGui::InputInt("Samples per frame", &lightmapSystem.BakingSettings.SamplesPerFrame);

					static VkDescriptorSet PreviewImage = NULL;

					if (ImGui::Button("Generate UV2"))
					{
						GenerateAndPackLightmaps(lightmapSystem, cpuScene);
						UploadLightmapMeshesToGPU(lightmapSystem, device, cpuScene, scene);

						// TODO: make imgui image preview work normally
						TextureVulkan* vktex = static_cast<TextureVulkan*>(lightmapSystem.Atlas.Lightmap);
						PreviewImage = ImGui_ImplVulkan_AddTexture(vktex->_Sampler, vktex->_View, vktex->_Layout);
					}

					if (ImGui::Button("Bake"))
					{
						lightmapSystem.BakingRequested = true;
						lightmapSystem.BakingData.AccumulatedSamples = 0;
					}

					if (lightmapSystem.BakingRequested)
					{
						ImGui::ProgressBar((float)lightmapSystem.BakingData.AccumulatedSamples / lightmapSystem.BakingSettings.RequestedSamples);
					}

					if (lightmapSystem.Atlas.Lightmap != nullptr)
					{
						ImGui::Image(PreviewImage, ImVec2(200, 200));
					}
				}
				ImGui::End();
			}

			// irradiance volume
			{
				if (ImGui::Begin("Irradiance Volume"))
				{
					ImGui::SliderFloat3("Position", (float*)&scene->IrradianceVolumes[0].Position, -10, 10);
					ImGui::SliderFloat3("Extent", (float*)&scene->IrradianceVolumes[0].Extent, -10, 10);
					ImGui::SliderInt3("Count", (int*)&scene->IrradianceVolumes[0].ProbesCount, 2, 8);
					ImGui::SliderFloat3("TestPoint", (float*)&scene->IrradianceVolumes[0].TestPoint, -5, 5);

					if (ImGui::Button("Compute"))
					{
						ComputeIrradianceVolume = true;
					}

					Matrix Transform;
					Transform.Scale(scene->IrradianceVolumes[0].Extent);
					Transform.Translate(scene->IrradianceVolumes[0].Position);
					debugRender.AddBox(Transform, Vector4(1, 1, 1, 0.1f));
				}
				ImGui::End();
			}

			// TODO: move to appropriate place
			// TODO: object properties editor
			// decal editor
			{
				if (ImGui::Begin("Decal"))
				{
					static Vector3 Pos{0};
					static Vector3 Scale{100};

					ImGui::SliderFloat3("Position", (float*)&Pos, -500, +500);
					ImGui::SliderFloat3("Scale", (float*)&Scale, 1, 500);

					Matrix Model;
					Model.Scale(Scale);
					Model.Translate(Pos);

					scene->Decals[0].Model = Model;
					scene->Decals[0].ModelInverse = Model.GetInverted();

					debugRender.AddBox(Model, Vector4(1, 1, 1, 0.1f));
				}
				ImGui::End();
			}

			// TODO: move to appropriate place
			// TODO: object properties editor
			// light editor
			{
				if (ImGui::Begin("Light"))
				{
					fixed_vector<int, 16> LightsToDelete;

					for (int i = 0; i < scene->Lights.size(); i++)
					{
						GPULight& Light = scene->Lights[i];

						char Label[256]{0};
						snprintf(Label, 256, "%i", i);
						if (ImGui::CollapsingHeader(Label))
						{
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
					}

					for (int LightId : LightsToDelete)
					{
						// TODO: think about cleaning up render resources for light source
						scene->Lights.erase(scene->Lights.begin() + LightId);
					}

					if (ImGui::Button("+"))
					{
						GPULight NewLight{{}, {0,1,0,0}, {1,1,1,1}, LightType::Point, 100, 0};
						scene->Lights.push_back(NewLight);
					}
				}
				ImGui::End();
			}
		}

		if (Window.Swapchain->IsOutdated)
		{
			device->QueueWaitIdle();
			Window.RecreateSwapchain();
		}

		RenderView View {
			.Swapchain = Window.Swapchain,
			.OutputSize = Window.Size,
			.CameraCur = camera,
			.CameraPrev = cameraPrev,
			.DebugRender = &debugRender,
		};

		cameraPrev = camera;

		renderGraph.Clear();

		{
			PROFILE_CPU(CpuCounter_RenderGraphCreate);

			UploadGPUSceneRG(renderGraph);

			if (render_cvar.GetValue() == 0)
			{
				if (lightmapSystem.BakingRequested)
				{
					PrepareAtlasForLightmapBaking(renderGraph, lightmapSystem);
					BakeLightmapPathTraced(renderGraph, lightmapSystem);
				}

				if (scene->IrradianceVolumes[0].ProbesBuffer == nullptr || ComputeIrradianceVolume)
				{
					RenderIrradianceProbes(renderGraph, View, scene->IrradianceVolumes[0]);
					ComputeIrradianceVolume = false;
				}

				RenderDeferred(renderGraph, View, DeferredContext);
			}
			else
			{
				RenderPathTraced(renderGraph, View);
			}
		}

		renderGraph.Execute(Window.Swapchain); // TODO: move swapchain handling out of rendergraph
		// TODO: allow debug overlay to be rendered afterwards

		// std::string graphviz = renderGraph.ExportGraphviz();
		// printf("%s\n", graphviz.c_str());
		// return 0;

		if (!IsDebugConsoleFocused())
		{
			auto keyboard = SDL_GetKeyboardState(NULL);
			if (keyboard[SDL_SCANCODE_DOWN]) camera.Rot += Columbus::Vector3(5,0,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_UP]) camera.Rot += Columbus::Vector3(-5,0,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_LEFT]) camera.Rot += Columbus::Vector3(0,5,0) * DeltaTime * 20;
			if (keyboard[SDL_SCANCODE_RIGHT]) camera.Rot += Columbus::Vector3(0,-5,0) * DeltaTime * 20;

			if (keyboard[SDL_SCANCODE_W]) camera.Pos += camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_S]) camera.Pos -= camera.Direction() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_D]) camera.Pos += camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_A]) camera.Pos -= camera.Right() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LSHIFT]) camera.Pos += camera.Up() * DeltaTime * CameraSpeed;
			if (keyboard[SDL_SCANCODE_LCTRL]) camera.Pos -= camera.Up() * DeltaTime * CameraSpeed;
		}

		// TODO:
		//std::this_thread::sleep_for(std::chrono::milliseconds(3));

		device->EndFrame();
	}

	VK_CHECK(vkQueueWaitIdle(*device->_ComputeQueue));
	VK_CHECK(vkDeviceWaitIdle(device->_Device));

	//SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
