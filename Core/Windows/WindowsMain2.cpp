#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Graphics/Core/GraphicsCore.h"
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
#include "Editor/CommonUI.h"
#include <vulkan/vulkan.h>
#include <Core/Core.h>
#include <Core/CVar.h>
#include <Core/Reflection.h>
#include "Scene/Project.h"
#include <Graphics/RenderGraph.h>
#include <Graphics/RenderPasses/RenderPasses.h>
#include <Graphics/Lightmaps.h>
#include <Graphics/World.h>
#include <Profiling/Profiling.h>
#include <Math/Box.h>
#include "Scene/AssetImport.h"
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

DECLARE_CPU_PROFILING_COUNTER(Counter_TotalCPU);
DECLARE_CPU_PROFILING_COUNTER(CpuCounter_RenderGraphCreate);

IMPLEMENT_CPU_PROFILING_COUNTER("Total CPU", "CPU", Counter_TotalCPU);
IMPLEMENT_CPU_PROFILING_COUNTER("RG Add", "RenderGraph", CpuCounter_RenderGraphCreate);

// TODO: make it normal
int SelectedObject = -1;
const bool DoPlayer = false;

struct Player
{
	EngineWorld& World;

	int MeshId;
	//GameObjectId Id;
	Rigidbody* RB;

	Player(EngineWorld& World) : World(World)
		//, Id(-1)
	{}

	void Load();
	void Spawn();

	void HandleInput();

	void PrePhysics();
	void PostPhysics();
};

void Player::Load()
{
	//if (!DoPlayer) return;

	//Model model;
	//model.Load("Data/Meshes/Sphere.obj");
	////model.Load("Data/Meshes/Hercules.cmf");
	//model.RecalculateTangents();
	//MeshId = World.LoadMesh(model);

	//SubModel a = model.GetSubModel(0);

	//Vector3 Center;
	//float Radius = 0;
	//{
	//	for (int i = 0; i < a.VerticesCount; i++)
	//	{
	//		Center += a.Positions[i];
	//	}

	//	Center /= a.VerticesCount;

	//	for (int i = 0; i < a.VerticesCount; i++)
	//	{
	//		Radius = Math::Max(a.Positions[i].Distance(Center), Radius);
	//	}
	//}

	//PhysicsShapeConvexHull* Hull = new PhysicsShapeConvexHull((float*)a.Positions, a.VerticesCount);
	//PhysicsShapeSphere* Sphere = new PhysicsShapeSphere(Radius);

	//RB = new Rigidbody(Sphere);
	//RB->SetStatic(false);
}

void Player::Spawn()
{
	if (!DoPlayer) return;

//	if (Id != -1)
//		return; // already spawned

	//Id = World.CreateGameObject("Player", MeshId);
	//World.GameObjects[Id].Trans.Position = Vector3(0, 100, 0);
	//World.GameObjects[Id].Trans.Update();

	//RB->SetTransform(World.GameObjects[Id].Trans);

	//World.Physics.AddRigidbody(RB);
}

void Player::HandleInput()
{
	if (!DoPlayer) return;

	auto keyboard = SDL_GetKeyboardState(NULL);
	if (keyboard[SDL_SCANCODE_I]) RB->ApplyCentralImpulse(Vector3(0, 0, +1));
	if (keyboard[SDL_SCANCODE_J]) RB->ApplyCentralImpulse(Vector3(+1, 0, 0));
	if (keyboard[SDL_SCANCODE_K]) RB->ApplyCentralImpulse(Vector3(0, 0, -1));
	if (keyboard[SDL_SCANCODE_L]) RB->ApplyCentralImpulse(Vector3(-1, 0, 0));
}

void Player::PrePhysics()
{
	if (!DoPlayer) return;

	//if (Id != -1)
	{
		//RB->SetTransform(World.GameObjects[Id].Trans);
	}
}

void Player::PostPhysics()
{
	if (!DoPlayer) return;

	//if (Id != -1)
	{
		//World.GameObjects[Id].Trans = RB->GetTransform();
		//World.GameObjects[Id].Trans.Update();
	}
}

// TODO: move to appropriate place
static void DrawGameViewportWindow(Texture2* FinalTexture, EngineWorld& World, Matrix ViewMatrix, Matrix ProjectionMatrix, iVector2& InOutViewSize, bool& OutWindowHover, bool& OutViewportFocus, Vector2& OutRelativeMousePosition)
{
	OutWindowHover = false;

	static ImGuizmo::OPERATION OperationMode = ImGuizmo::OPERATION::TRANSLATE;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("Viewport"))
	{
		OutWindowHover = ImGui::IsWindowHovered();
		OutViewportFocus = ImGui::IsWindowFocused();

		float BarHeight = 20;
		ImGui::Button("Bake GI", ImVec2(0, BarHeight)); ImGui::SameLine();
		ImGui::RadioButton("T", (int*)& OperationMode, ImGuizmo::OPERATION::TRANSLATE); ImGui::SameLine();
		ImGui::RadioButton("R", (int*)&OperationMode, ImGuizmo::OPERATION::ROTATE); ImGui::SameLine();
		ImGui::RadioButton("S", (int*)&OperationMode, ImGuizmo::OPERATION::SCALE);

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

		ImGuizmo::SetRect(WindowPos.x, WindowPos.y + Cursor.y, viewportSize.x, viewportSize.y);
		ImGuizmo::SetDrawlist();

		if (SelectedObject != -1)
		{
			// TODO: proper scene graph and transform management

			/*Transform& Trans = World.GameObjects[SelectedObject].Trans;

			Matrix TestMatrix = Trans.GetMatrix().GetTransposed();
			ImGuizmo::Manipulate(&ViewMatrix.M[0][0], &ProjectionMatrix.M[0][0], OperationMode, ImGuizmo::MODE::LOCAL, &TestMatrix.M[0][0]);

			Vector3 Euler;
			TestMatrix.Transpose();
			TestMatrix.DecomposeTransform(Trans.Position, Euler, Trans.Scale);
			Trans.Rotation = Quaternion(Euler);*/
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(1);
}

static void AddWorldCollision(EngineWorld& World)
{
	return;
	// create rigidbodies for the level
	/*for (int i = 0; i < (int)World.GameObjects.size(); i++)
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
	}*/
}

// all the data about current editor context
struct EngineEditor
{
	// list of potential things:
	// - currently picked object/mouse picking logic
	// - vieweport hover states
	// - registration of all callbacks for menu buttons

	// - states to update baking systems and lock editor
};


enum class TestEnum
{
	One,
	Two = 15,
	Three,
	Four = 123
};

// SHOULD BE CODEGEN with include
template <> const Reflection::Enum* Reflection::FindEnum<TestEnum>();



// SHOULD BE CODEGEN
#if 1
struct TestEnum__Registration
{
	Reflection::Enum EnumData;

	TestEnum__Registration()
	{
		EnumData.Name = "TestEnum";
		EnumData.Fields.push_back(Reflection::EnumField{ .Name = "One",   .Value = (int)TestEnum::One,   .Index = 0 });
		EnumData.Fields.push_back(Reflection::EnumField{ .Name = "Two",   .Value = (int)TestEnum::Two,   .Index = 1 });
		EnumData.Fields.push_back(Reflection::EnumField{ .Name = "Three", .Value = (int)TestEnum::Three, .Index = 2 });
		EnumData.Fields.push_back(Reflection::EnumField{ .Name = "Four",  .Value = (int)TestEnum::Four,  .Index = 3 });
	}
} TestEnum__Registration_Instance;

template <>
const Reflection::Enum* Reflection::FindEnum<TestEnum>()
{
	return &TestEnum__Registration_Instance.EnumData;
}
#endif



bool EnumComboBox(const char* Label, const Reflection::Enum* Enum, int* Value)
{
	const Reflection::EnumField* Field = Enum->FindFieldByValue(*Value);
	int Idx = Field->Index;

	bool Result = ImGui::Combo(Label, &Idx, [](void* data, int idx, const char** out_text) -> bool
	{
		*out_text = ((Reflection::Enum*)data)->Fields[idx].Name;
		return true;
	}, (void*)Enum, Enum->Fields.size());

	*Value = Enum->Fields[Idx].Value;
	return Result;
}

template <typename T>
bool EnumComboBox(const char* Label, T* Value)
{
	return EnumComboBox(Label, Reflection::FindEnum<T>(), (int*)Value);
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
// Scene scene = LoadScene(...); // loads all entities
// AThing* thing = scene.AddThing(...);
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
// 4. + Static reflection
// 5. Entities
// 6. TaskGraph
// 7. + CVar system (+console)
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
//				+ sphere
//				- capsule
//			+ importance sampling
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
//			- translucency
//             - frosted glass
//          - DDGI
//			- RT reflections (TODO: denoiser)
//          + RTGI (first bounce, diffuse)
//			- GI1.0
//			- simple billboard particles render
//			+ upscaling (FSR1)
//          + TAA
//          - FSR2
//		3. Common
//			+ PBR atmosphere rendering
//			- filmic camera
//			+ filmic HDR tonemapper
//			- render image export from every stage of a graph
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
int main(int argc, char** argv)
{
	// TOOD: create import unit tests
	//Assets::ImportLevel("C:/Users/Columbus/Documents/src/TestProject/Data/TestLevel/TestLevel.gltf", "C:/Users/Columbus/Documents/src/TestProject/Data/TestImport/Level.gltf");
	//return 0;

	// TODO: create other unit tests

	// 3d texture gen
	if (0)
	{
		char Volume[32][32][32][4]; // rgba8 32x32x32

		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 32; j++)
			{
				for (int k = 0; k < 32; k++)
				{
					char pixel[4]{
						(char)((i / 32.0f) * 255),
						(char)((j / 32.0f) * 255),
						(char)((k / 32.0f) * 255),
						255,
					};

					memcpy(Volume[i][j][k], pixel, 4);
				}
			}
		}

		Image Img;
		Img.Type = ImageType::Image3D;
		Img.Exist = true;
		Img.Data = (u8*)Volume;
		Img.MipMaps = 1;
		Img.Width = 32;
		Img.Height = 32;
		Img.Depth = 32;
		Img.Format = TextureFormat::RGBA8;

		DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/volume.dds", "wb");
		ImageUtils::ImageSaveToStreamDDS(Stream, Img);
	}

	// Test DDS Load/Save
	if (0)
	{
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgb.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgb_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt1.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt3.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt3_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt5.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/dxt5_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/bc7.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/bc7_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgb_mips.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgb_mips_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba_mips.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba_mips_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/cubemap_mips.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/cubemap_mips_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba16f.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba16f_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba32f.dds");
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/load_test/rgba32f_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
	}

	// Compression test
	if (0)
	{
		ImageCompression::InitImageCompression();

		const bool bCompressFromDDS = false;

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT5,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb_dxt5_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::BC7,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb_bc7_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT5,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_dxt5_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (bCompressFromDDS)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::BC7,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_bc7_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb.jpg");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb_jpg_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba.png");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_png_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb.jpg");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::BC7,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgb_jpg_bc7_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_mips.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgab_mips_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/volume.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/volume_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/cubemap_rgba_mips.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::DXT1,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/cubemap_dxt1_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba16f.dds");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::BC6H,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba16f_bc6h_result.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/Abandoned.hdr");
			Image CompressedImage;

			ImageCompression::CompressionParams Params{
				.Format = TextureFormat::BC6H,
			};

			ImageCompression::CompressImage(Img, CompressedImage, Params);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/Abandoned_bc6h.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, CompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/rgba_dxt1_result.dds");
			Image DecompressedImage;

			ImageCompression::DecompressImage(Img, DecompressedImage);
			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/comp_test/decompress_rgba_dxt1.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, DecompressedImage);
		}

		if (0)
		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/mip_test/rgba.dds");
			Image DstImg;

			ImageMips::GenerateImageMips(Img, DstImg, {});
			ImageCompression::CompressImage(DstImg, Img, ImageCompression::CompressionParams{
				.Format = TextureFormat::DXT1
			});

			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/mip_test/rgba_mipgen.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}

		{
			Image Img;
			Img.LoadFromFile("C:/Users/Columbus/Downloads/tmp/mip_test/volume.dds");
			Image DstImg;

			ImageMips::GenerateImageMips(Img, DstImg, {});
			ImageCompression::CompressImage(DstImg, Img, ImageCompression::CompressionParams{
				.Format = TextureFormat::DXT1
			});

			DataStream Stream = DataStream::CreateFromFile("C:/Users/Columbus/Downloads/tmp/mip_test/volume_mipgen.dds", "wb");
			ImageUtils::ImageSaveToStreamDDS(Stream, Img);
		}
	}

	//return 0;
	InitializeEngine(argc, argv);

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

	EngineWorld World(device);
	World.LoadLevelGLTF(SceneLoadPath);
	// World.SceneGPU = SPtr<GPUScene>(GPUScene::CreateGPUScene(device)); // empty level

	// World.RegisterSystem<SPlayerSystem>();
	// World.RegisterSystem<SGaussianSystem>();
	// World.RegisterSystem<SLightBakingSystem>();

	AddWorldCollision(World);

	Player player(World);
	player.Load();

	auto renderGraph = RenderGraph(device, World.SceneGPU);
	WindowVulkan Window(instance, device);
	DeferredRenderContext DeferredContext(device);

	World.MainView.OutputSize = Window.GetSize();

	bool bViewportHover = false;
	bool bViewportFocused = false;
	bool bImguizmoHover = false;
	Vector2 ViewportMousePos;
	Vector2 ViewportSize;

	DebugUI::Context* UiContext = DebugUI::Create(&Window);

	IrradianceVolume Volume;
	Volume.ProbesCount = {8,8,8};
	Volume.Extent = {3,3,3};
	World.SceneGPU->IrradianceVolumes.push_back(Volume);

	bool ComputeIrradianceVolume = false;

	bool running = true;
	while (running)
	{
		GFrameNumber++;
		device->BeginFrame();
		World.BeginFrame();
		DebugUI::BeginFrame(UiContext);
		PROFILE_CPU(Counter_TotalCPU);

		float DeltaTime = (float)timer.Elapsed();
		timer.Reset();

		// TODO:
		camera.Perspective(60, (float)World.MainView.OutputSize.X / (float)World.MainView.OutputSize.Y, 0.1f, 5000.f);
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

		player.PrePhysics();
		World.Update(DeltaTime);
		player.PostPhysics();

		// mouse picking
		if (bViewportHover && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) && !bImguizmoHover)
		{
		}

		// TODO: common object selection interface
		/*if (SelectedObject != -1)
		{
			const int MeshId = World.GameObjects[SelectedObject].MeshId;
			const Box Bounding = World.Meshes[MeshId]->BoundingBox;
			Matrix Transform = World.GameObjects[SelectedObject].Trans.GetMatrix();

			Transform.Scale(Bounding.CalcSize());
			Transform.Translate(Bounding.CalcCenter());

			World.MainView.DebugRender.AddBox(Transform, Vector4(0, 0.3f, 0, 0.3f));
		}*/

		// UI Prepare
		{
			DebugUI::DrawMainLayout();
		}

		// rendergraph stuff
		{
			RenderGraphTextureRef FinalTexture = -1;
			renderGraph.Clear();

			// setup render passes
			{
				PROFILE_CPU(CpuCounter_RenderGraphCreate);

				World.MainView.RenderSize = World.MainView.OutputSize; // TODO: think of this logic
				UploadGPUSceneRG(renderGraph, World.MainView);

				if (render_cvar.GetValue() == 0)
				{
					if (World.Lightmaps.BakingRequested)
					{
						PrepareAtlasForLightmapBaking(renderGraph, World.Lightmaps);
						BakeLightmapPathTraced(renderGraph, World.Lightmaps);
					}

					if (ComputeIrradianceVolume)
					{
						RenderIrradianceProbes(renderGraph, World.MainView, World.SceneGPU->IrradianceVolumes[0]);
						ComputeIrradianceVolume = false;
					}

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

					// UI
					{
						{
							char Buf[256]{};
							snprintf(Buf, 256, "Columbus Engine (Vulkan) - %s", GCurrentProject ? GCurrentProject->ProjectName.c_str() : "No project");
							SDL_SetWindowTitle(Window.Window, Buf);
						}

						ImGui::ShowDemoWindow();
						ShowDebugConsole();
						ShowRenderGraphVisualiser(renderGraph);

						DebugUI::ShowScreenshotSaveWindow(World.MainView);

						DebugUI::ShowProjectSettingsWindow();
						DrawGameViewportWindow(renderGraph.GetTextureAfterExecution(FinalTexture), World, camera.GetViewMatrix(), camera.GetProjectionMatrix(), World.MainView.OutputSize, bViewportHover, bViewportFocused, ViewportMousePos);
						DebugUI::ShowMeshesWindow(World);
						DebugUI::ShowDecalsWindow(World);
						DebugUI::ShowLightsWindow(World);
						DebugUI::ShowMaterialsWindow(World);
						DebugUI::ShowIrradianceWindow(World);
						DebugUI::ShowLightmapWindow(World);
						Editor::TickAllModalWindows();

						if (ImGui::Begin("Sky"))
						{
							//// scaterring coefficients: w parameter unused for GPU alignments
							//Vector4 BetaRayleigh = Vector4(3.8e-6f, 13.5e-6f, 33.1e-6f, 0);
							//Vector4 BetaMie = Vector4(21e-6f, 21e-6f, 21e-6f, 0);
							//Vector4 BetaOzone = Vector4(2.04e-5f, 4.97e-5f, 1.95e-6f, 0);

							ImGui::SliderInt("Samples", &World.Sky.Samples, 1, 16);
							ImGui::SliderInt("Light Samples", &World.Sky.LightSamples, 1, 16);

							ImGui::SliderFloat("Planet Radius (km)", &World.Sky.PlanetRadiusKm, 0.0f, 10000.0f);
							ImGui::SliderFloat("Atmosphere Height (km)", &World.Sky.AtmosphereHeightKm, 0.0f, 1000.0f);
							ImGui::SliderFloat("Rayleight Height (km)", &World.Sky.RayleightHeightKm, 0.0f, 10.0f);
							ImGui::SliderFloat("Mie Height (km)", &World.Sky.MieHeight, 0.0f, 4.0f);
							ImGui::SliderFloat("Ozone Peak (km)", &World.Sky.OzonePeak, 0.0f, 100.0f);
							ImGui::SliderFloat("Ozon Falloff (km)", &World.Sky.OzoneFalloff, 0.0f, 10.0f);

							ImGui::SliderFloat("Sun Illuminance", &World.Sky.SunIlluminance, 0.0f, 400000.0f);
							ImGui::SliderFloat("Moon Illuminance", &World.Sky.MoonIlluminance, 0.0f, 10.0f);
							ImGui::SliderFloat("Space Illuminance", &World.Sky.SpaceIlluminance, 0.0f, 1.0f);
							ImGui::SliderFloat("Expoure", &World.Sky.Exposure, 0.0f, 100.0f);

							static TestEnum asd = TestEnum::One;
							EnumComboBox<TestEnum>("Test", &asd);

							ImGui::LabelText("Test Value", "%i", (int)asd);
						}
						ImGui::End();

						if (ImGui::Begin("Camera"))
						{
							ImGui::InputFloat3("Camera Position", (float*)&camera.Pos);
							ImGui::InputFloat3("Camera Rotation", (float*)&camera.Rot);

							ImGui::Checkbox("Enable DoF", &camera.EnableDoF);
							ImGui::SliderFloat("F-Stop", &camera.FStop, 0.1f, 10.0f);
							ImGui::SliderFloat("Focus", &camera.FocusDistance, 0.01f, 100.0f);
							ImGui::SliderFloat("SensorSize", &camera.SensorSize, 0.01f, 10.0f);

							ImGui::Checkbox("Enable Vignette", &camera.EnableVignette);
							ImGui::SliderFloat("Vignette", &camera.Vignette, 0.0f, 2.0f);

							ImGui::Checkbox("Enable Grain", &camera.EnableGrain);
							ImGui::SliderFloat("Grain Scale", &camera.GrainScale, 0.01f, 100.0f);
							ImGui::SliderFloat("Grain Amount", &camera.GrainAmount, 0.0f, 2.0f);
						}
						ImGui::End();

						if (ImGui::Begin("Irradiance"))
						{
							if (ImGui::Button("Compute"))
							{
								ComputeIrradianceVolume = true;
							}
						}
						ImGui::End();
					}

					ViewportSize = World.MainView.OutputSize;
					bImguizmoHover = ImGuizmo::IsOver();
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

	//SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
