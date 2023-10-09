#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Graphics/Core/GraphicsCore.h"
#include "Editor/CommonUI.h"
#include "Graphics/Camera.h"
#include "Graphics/GPUScene.h"
#include "Graphics/IrradianceVolume.h"
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
#include <vulkan/vulkan.h>
#include <Core/Core.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/RenderPasses/RenderPasses.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <Graphics/Vulkan/InstanceVulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>

#include <vector>
#include <algorithm>

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
		vkDestroySwapchainKHR(Device->_Device, Swapchain->swapChain, nullptr);
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

/*class ImguiPass : public RenderPass
{
public:
	ImguiPass(WindowVulkan& Window) : Window(Window), RenderPass("ImGUI Pass")
	{
		IsGraphicsPass = true;
		ClearColor = false;
		AddOutputRenderTarget(AttachmentDesc(FinalColorOutput, AttachmentType::Color, AttachmentLoadOp::Load, TextureFormat::BGRA8SRGB));

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		// Editor::ApplyDarkTheme();
		io.DisplaySize = {1280,720};
	}

	virtual void Setup(RenderGraphContext& Context) override
	{
		// init imgui
		ImGui_ImplSDL2_InitForVulkan(Window.Window);
		ImGui_ImplVulkan_InitInfo imguiVk{};
		imguiVk.Instance = Window.Instance.instance;
		imguiVk.PhysicalDevice = Context.Device->_PhysicalDevice;
		imguiVk.Device = Context.Device->_Device;
		imguiVk.QueueFamily = Context.Device->_FamilyIndex;
		imguiVk.Queue = *Context.Device->_ComputeQueue;
		imguiVk.DescriptorPool = Context.Device->_DescriptorPool;
		imguiVk.MinImageCount = Window.Swapchain->minImageCount;
		imguiVk.ImageCount = Window.Swapchain->imageCount;
		ImGui_ImplVulkan_Init(&imguiVk, Context.VulkanRenderPass);

		{
			auto CommandBuffer = Context.Device->CreateCommandBufferShared();
			CommandBuffer->Reset();
			CommandBuffer->Begin();
			ImGui_ImplVulkan_CreateFontsTexture(CommandBuffer->_CmdBuf);

			VkSubmitInfo end_info = {};
			end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			end_info.commandBufferCount = 1;
			end_info.pCommandBuffers = &CommandBuffer->_CmdBuf;
			vkEndCommandBuffer(CommandBuffer->_CmdBuf);
			vkQueueSubmit(*Context.Device->_ComputeQueue, 1, &end_info, VK_NULL_HANDLE);

			vkDeviceWaitIdle(Context.Device->_Device);
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		}
	}

	virtual TExecutionFunc Execute2(RenderGraphContext& Context) override
	{
		ImGui::NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(Window.Window);

		return [](RenderGraphContext& Context)
		{
			ImGui::ShowDemoWindow();
			ImGui::Render();
			ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Context.CommandBuffer->_CmdBuf);
		};
	}
private:
	WindowVulkan& Window;
};*/

Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data)
{
	BufferDesc desc;
	desc.BindFlags = BufferType::UAV;
	desc.Size = size;
	desc.UsedInAccelerationStructure = usedInAS;
	return device->CreateBuffer(desc, data);
}

SPtr<GPUScene> LoadScene(SPtr<DeviceVulkan> Device, Camera DefaultCamera, const std::string& Filename)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	if (!loader.LoadASCIIFromFile(&model, &err, &warn, Filename))
	{
		Log::Fatal("Couldn't load scene, %s", Filename.c_str());
	}

	SPtr<GPUScene> Scene = std::make_shared<GPUScene>();

	for (auto& texture : model.textures)
	{
		auto& image = model.images[texture.source];

		Image img;
		img.FromMemory(image.image.data(), image.image.size(), image.width, image.height);

		auto tex = Device->CreateTexture(img);
		Device->SetDebugName(tex, texture.name.c_str());
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
			Device->SetDebugName(BLAS, mesh.name.c_str());

			int matid = -1;

			if (primitive.material > -1)
			{
				auto mat = model.materials[primitive.material];
				matid = mat.pbrMetallicRoughness.baseColorTexture.index;
			}

			{
				materialBuffer = CreateMeshBuffer(Device, sizeof(int), true, &matid);
				Device->SetDebugName(materialBuffer, (mesh.name + " (Material)").c_str());
			}

			GPUSceneMesh Mesh;
			Mesh.BLAS = BLAS;
			Mesh.Transform = Matrix(1);
			Mesh.Vertices = vertexBuffer;
			Mesh.Indices = indexBuffer;
			Mesh.UVs = uvBuffer;
			Mesh.Normals = normalBuffer;
			Mesh.Material = materialBuffer;
			Mesh.VertexCount = verticesCount;
			Mesh.IndicesCount = indicesCount;

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

	Scene->Lights.push_back(GPULight{{}, {0,1,0,0}, {1,1,1,0}, 0}); // directional
	// Scene->Lights.push_back(GPULight{{0,200,0,0}, {}, {50,0,0,0}, 1}); // point

	return Scene;
}

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
//		- global/frame-local resources
//		+ swapchain resize (and pipeline dynamic parameters)
//		- resource aliasing
//		- resource visualization
//		- diagnostic information
//		- resource type validation
//		- RW passes
//		- storage/sampled images transitions
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
//			- ray-traced shadows with variable penumbra
//			- directional lighting pass (lights + shadows)
//			- ray-traced spherical harmonics/ambient cubes (DDGI-ish solution)
//			- RTAO?
//			- volumetrics and OpenVDB
//			- ray-traced translucency
//			- DDGI+RT reflections
//			- simple billboard particles render
//		3. Common
//			- PBR atmosphere rendering
//			- filmic camera
//			- filmic HDR tonemapper
//			- render image export (!!!)
//			- decals
//			- subsurface scattering
//			- IES light profiles
//			+ shader include files
//			- fix descriptor set duplication
//			- HLSL2021 instead of GLSL
//			- make DebugRender work
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
	Columbus::Timer timer;
	camera.Pos = { 0, 300, 104 };
	camera.Rot = { 0, -70, 0 };
	camera.Perspective(45, 1280.f/720.f, 1.f, 5000.f);
	float CameraSpeed = 200;

	// SPtr<GPUScene> scene = std::make_shared<GPUScene>();

	Columbus::InstanceVulkan instance;
	auto device = instance.CreateDevice();
	// auto scene = LoadScene(device, camera, "D:/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
	auto scene = LoadScene(device, camera, "/home/columbus/assets/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf");
	// auto scene = LoadScene(device, camera, "/home/columbus/assets/glTF-Sample-Models-master/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
	// auto scene = LoadScene(device, camera, "/home/columbus/assets/cubes.gltf");
	auto renderGraph = RenderGraph(device, scene);
	WindowVulkan Window(instance, device);

	IrradianceVolume Volume;
	Volume.Position = { 1000, 50, -250 };
	Volume.ProbesCount = { 5, 4, 5 };
	Volume.Extent = { 250, 320, 500 };

	bool running = true;
	while (running)
	{
		float DeltaTime = timer.Elapsed();
		timer.Reset();

		camera.Update();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			// ImGui_ImplSDL2_ProcessEvent(&event);
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

		if (Window.Swapchain->IsOutdated)
		{
			device->QueueWaitIdle();
			Window.RecreateSwapchain();
		}

		renderGraph.Clear();
		RenderPathTraced(renderGraph, camera, Window.Size);
		// RenderDeferred(renderGraph, camera, Window.Size);
		renderGraph.Execute(Window.Swapchain);

		// std::string graphviz = renderGraph.ExportGraphviz();
		// printf("%s\n", graphviz.c_str());
		// return 0;

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

	VK_CHECK(vkQueueWaitIdle(*device->_ComputeQueue));
	VK_CHECK(vkDeviceWaitIdle(device->_Device));

	//vkDestroySwapchainKHR(device->_Device, swapchain->swapChain, nullptr);
	//vkDestroySurfaceKHR(instance.instance, surface, nullptr);
	//SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
