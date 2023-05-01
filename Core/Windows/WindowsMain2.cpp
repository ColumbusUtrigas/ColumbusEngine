#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Core/Stacktrace.h"
#include "Core/Timer.h"
#include "Editor/CommonUI.h"
#include "Graphics/Buffer.h"
#include "Graphics/Camera.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RayTracingPipeline.h"
#include "Graphics/Texture.h"
#include "Graphics/Types.h"
#include "Graphics/Vulkan/AccelerationStructureVulkan.h"
#include "Graphics/Vulkan/BufferVulkan.h"
#include "Graphics/Vulkan/RayTracingPipelineVulkan.h"
#include "Input/Events.h"
#include "Input/Input.h"
#include "Math/Matrix.h"
#include "Math/Plane.h"
#include "Math/Quaternion.h"
#include "Math/Vector3.h"
#include "SDL_events.h"
#include "SDL_keyboard.h"
#include "SDL_scancode.h"
#include "Scene/Transform.h"
#include "Lib/imgui/imgui.h"
#include "Lib/imgui/backends/imgui_impl_vulkan.h"
#include "Lib/imgui/backends/imgui_impl_sdl2.h"
#include "System/File.h"
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <csignal>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <Graphics/Vulkan/InstanceVulkan.h>
#include <Graphics/Vulkan/DeviceVulkan.h>

#include <vector>
#include <algorithm>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <Lib/tinygltf/tiny_gltf.h>

std::string srcScreenVert = R"(#version 460 core
layout (location = 0) out vec2 texcoords;

const vec3 verts[3] = vec3[](
    vec3(-3, -1, 0),
    vec3(1, 3, 0),
    vec3(1, -1, 0)
);

void main() {
    gl_Position = vec4(verts[gl_VertexIndex], 1);
    texcoords = 0.5 * gl_Position.xy + vec2(0.5);
}
)";

std::string srcScreenFrag = R"(#version 460 core
    layout(location = 0) in vec2 texcoords;

    layout(binding = 0, set = 0, rgba32f) uniform image2D img;

    layout(location = 0) out vec4 RT0;

    layout(push_constant) uniform params
    {
        uint frameNumber;
    } Parameters;

    void main() {
        vec3 color = imageLoad(img, ivec2(texcoords * vec2(1280,720))).rgb / float(Parameters.frameNumber);
        RT0 = vec4(color, 1.0);
    }
)";

const int MAX_FRAMES_IN_FLIGHT = 2;
using namespace Columbus;

struct Material2
{
    int TextureId;
};

struct Mesh2
{
    Buffer* VertexBuffer;
    Buffer* UvBuffer;
    Buffer* NormalBuffer;
    Buffer* IndexBuffer;
    Buffer* MaterialBuffer;
    AccelerationStructure* BLAS;

    static Buffer* CreateMeshBuffer(SPtr<DeviceVulkan> device, size_t size, bool usedInAS, const void* data)
    {
        BufferDesc desc;
        desc.BindFlags = BufferType::UAV;
        desc.Size = size;
        desc.UsedInAccelerationStructure = usedInAS;
        return device->CreateBuffer(desc, data);
    }

    Mesh2() {}

    Mesh2(SPtr<DeviceVulkan> device, const char* filename)
    {
        Model model;
        model.Load(filename);
        const SubModel& submodel = model.GetSubModel(0);

        std::vector<uint32_t> indices;

        if (submodel.Indexed)
        {
            for (int i = 0; i < submodel.IndicesCount; i++)
            {
                auto indicesArray = (uint16_t*)submodel.Indices;
                indices.push_back(indicesArray[i]);
            }
        }
        else
        {
            for (int i = 0; i < submodel.VerticesCount; i++)
            {
                indices.push_back(i);
            }
        }

        VertexBuffer = CreateMeshBuffer(device, submodel.VerticesCount * sizeof(Vector3), true, submodel.Positions);
        UvBuffer     = CreateMeshBuffer(device, submodel.VerticesCount * sizeof(Vector2), false, submodel.UVs);
        NormalBuffer = CreateMeshBuffer(device, submodel.VerticesCount * sizeof(Vector3), false, submodel.Normals);
        IndexBuffer  = CreateMeshBuffer(device, indices.size() * sizeof(uint32_t), true, indices.data());

        Columbus::AccelerationStructureDesc blasDesc;
        blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
        blasDesc.Vertices = VertexBuffer;
        blasDesc.Indices = IndexBuffer;
        blasDesc.VerticesCount = submodel.VerticesCount;
        blasDesc.IndicesCount = indices.size();
        BLAS = device->CreateAccelerationStructure(blasDesc);
    }
};

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
// TODO: think about extensions, non-game and non-editor scenarios
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

// Task priorities
// 1. RenderGraph
// 2. UI system (basic)
// 3. SceneGraph
// 4. Static reflection
// 5. ECS
// 6. TaskGraph

void CrashHandler(int signal)
{
    WriteStacktraceToLog();
    exit(1);
}

int main()
{
    std::signal(SIGSEGV, CrashHandler);

    VkSurfaceKHR surface;

    Columbus::InstanceVulkan instance;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
    SDL_Vulkan_CreateSurface(window, instance.instance, &surface);

    PFN_vkGetInstanceProcAddr vkInstanceGetProcAddr = (PFN_vkGetInstanceProcAddr)SDL_Vulkan_GetVkGetInstanceProcAddr();

    auto device = instance.CreateDevice();
    device->VkFunctions.LoadFunctions(instance.instance, vkInstanceGetProcAddr);

    auto swapchain = device->CreateSwapchain(surface);
    auto renderpass = device->CreateRenderPass(swapchain->swapChainImageFormat);
    device->CreateFramebuffers(swapchain, renderpass);
    auto cmdBuf = device->CreateCommandBuffer();
    auto fence = device->CreateFence(true);
    auto imageSemaphore = device->CreateSemaphore();
    auto accelerationStructureSemaphore = device->CreateSemaphore();
    auto submitSemaphore = device->CreateSemaphore();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    Editor::ApplyDarkTheme();

    // init imgui
    ImGui_ImplSDL2_InitForVulkan(window);
    ImGui_ImplVulkan_InitInfo imguiVk{};
    imguiVk.Instance = instance.instance;
    imguiVk.PhysicalDevice = device->_PhysicalDevice;
    imguiVk.Device = device->_Device;
    imguiVk.QueueFamily = device->_FamilyIndex;
    imguiVk.Queue = *device->_ComputeQueue;
    imguiVk.DescriptorPool = device->_DescriptorPool;
    imguiVk.MinImageCount = swapchain->minImageCount;
    imguiVk.ImageCount = swapchain->imageCount;
    ImGui_ImplVulkan_Init(&imguiVk, renderpass);

    {
        cmdBuf.Reset();
        cmdBuf.Begin();
        ImGui_ImplVulkan_CreateFontsTexture(cmdBuf._CmdBuf);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &cmdBuf._CmdBuf;
        vkEndCommandBuffer(cmdBuf._CmdBuf);
        vkQueueSubmit(*device->_ComputeQueue, 1, &end_info, VK_NULL_HANDLE);

        vkDeviceWaitIdle(device->_Device);
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    GraphicsPipelineDesc screenDesc {};
    screenDesc.Name = "Screen";
    screenDesc.rasterizerState.Cull = Columbus::CullMode::No;
    screenDesc.VS = std::make_shared<Columbus::ShaderStage>(srcScreenVert, "main", Columbus::ShaderType::Vertex, Columbus::ShaderLanguage::GLSL);
    screenDesc.PS = std::make_shared<Columbus::ShaderStage>(srcScreenFrag, "main", Columbus::ShaderType::Pixel, Columbus::ShaderLanguage::GLSL);
    auto screenPipeline = device->CreateGraphicsPipeline(screenDesc, renderpass);

    File rtShaderFile("./Data/Shaders/Rays.glsl", "rt");
    auto rtShaderSize = rtShaderFile.GetSize();
    char* rtShaderSource = new char[rtShaderSize+1];
    rtShaderSource[rtShaderSize] = '\0';
    rtShaderFile.Read(rtShaderSource, rtShaderSize, 1);

    Columbus::RayTracingPipelineDesc rtDesc{};
    rtDesc.Name = "RTXON";
    rtDesc.RayGen = std::make_shared<Columbus::ShaderStage>(rtShaderSource, "main", Columbus::ShaderType::Raygen, Columbus::ShaderLanguage::GLSL);
    rtDesc.Miss = std::make_shared<Columbus::ShaderStage>(rtShaderSource, "main", Columbus::ShaderType::Miss, Columbus::ShaderLanguage::GLSL);
    rtDesc.ClosestHit = std::make_shared<Columbus::ShaderStage>(rtShaderSource, "main", Columbus::ShaderType::ClosestHit, Columbus::ShaderLanguage::GLSL);
    rtDesc.MaxRecursionDepth = 1;
    auto rtPipeline = device->CreateRayTracingPipeline(rtDesc);

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    loader.LoadASCIIFromFile(&model, &err, &warn, "/home/columbus/assets/glTF-Sample-Models-master/2.0/Sponza/glTF/Sponza.gltf");
    // loader.LoadASCIIFromFile(&model, &err, &warn, "/home/columbus/assets/glTF-Sample-Models-master/2.0/SciFiHelmet/glTF/SciFiHelmet.gltf");
    // loader.LoadASCIIFromFile(&model, &err, &warn, "/home/columbus/assets/glTF-Sample-Models-master/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
    // loader.LoadASCIIFromFile(&model, &err, &warn, "/home/columbus/assets/test.gltf");

    std::vector<Texture2*> textures;
    std::vector<Mesh2> meshes;

    for (auto& texture : model.textures)
    {
        auto& image = model.images[texture.source];

        Image img;
        img.FromMemory(image.image.data(), image.image.size(), image.width, image.height);

        auto tex = device->CreateTexture(img);
        device->SetDebugName(tex, texture.name.c_str());
        textures.push_back(tex);
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

                indexBuffer = Mesh2::CreateMeshBuffer(device, indices.size() * sizeof(uint32_t), true, indices.data());
                device->SetDebugName(indexBuffer, (mesh.name + " (Indices)").c_str());
            }

            {
                auto accessor = model.accessors[primitive.attributes["POSITION"]];
                auto view = model.bufferViews[accessor.bufferView];
                auto buffer = model.buffers[view.buffer];
                auto offset = accessor.byteOffset + view.byteOffset;
                const void* data = buffer.data.data() + offset;
                verticesCount = accessor.count;

                vertexBuffer = Mesh2::CreateMeshBuffer(device, accessor.count * sizeof(Vector3), true, data);
                device->SetDebugName(vertexBuffer, (mesh.name + " (Vertices)").c_str());
            }

            {
                auto accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
                auto view = model.bufferViews[accessor.bufferView];
                auto buffer = model.buffers[view.buffer];
                auto offset = accessor.byteOffset + view.byteOffset;
                const void* data = buffer.data.data() + offset;
                verticesCount = accessor.count;

                uvBuffer = Mesh2::CreateMeshBuffer(device, accessor.count * sizeof(Vector2), true, data);
                device->SetDebugName(uvBuffer, (mesh.name + " (UVs)").c_str());
            }

            {
                auto accessor = model.accessors[primitive.attributes["NORMAL"]];
                auto view = model.bufferViews[accessor.bufferView];
                auto buffer = model.buffers[view.buffer];
                auto offset = accessor.byteOffset + view.byteOffset;
                const void* data = buffer.data.data() + offset;
                verticesCount = accessor.count;

                normalBuffer = Mesh2::CreateMeshBuffer(device, accessor.count * sizeof(Vector3), true, data);
                device->SetDebugName(normalBuffer, (mesh.name + " (Normals)").c_str());
            }

            Columbus::AccelerationStructureDesc blasDesc;
            blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
            blasDesc.Vertices = vertexBuffer;
            blasDesc.Indices = indexBuffer;
            blasDesc.VerticesCount = verticesCount;
            blasDesc.IndicesCount = indicesCount;
            auto BLAS = device->CreateAccelerationStructure(blasDesc);
            device->SetDebugName(BLAS, mesh.name.c_str());

            int matid = -1;

            if (primitive.material > -1)
            {
                auto mat = model.materials[primitive.material];
                matid = mat.pbrMetallicRoughness.baseColorTexture.index;
            }

            {
                materialBuffer = Mesh2::CreateMeshBuffer(device, sizeof(int), true, &matid);
                device->SetDebugName(materialBuffer, (mesh.name + " (Material)").c_str());
            }

            Mesh2 mesh;
            mesh.IndexBuffer = indexBuffer;
            mesh.VertexBuffer = vertexBuffer;
            mesh.UvBuffer = uvBuffer;
            mesh.NormalBuffer = normalBuffer;
            mesh.MaterialBuffer = materialBuffer;
            mesh.BLAS = BLAS;

            meshes.push_back(mesh);
        }
    }

    Columbus::AccelerationStructureDesc tlasDesc2;
    tlasDesc2.Type = Columbus::AccelerationStructureType::TLAS;
    tlasDesc2.Instances = {};
    for (auto& mesh : meshes)
    {
        tlasDesc2.Instances.push_back({ Columbus::Matrix(), mesh.BLAS });
    }
    auto tlas2 = device->CreateAccelerationStructure(tlasDesc2);

    TextureDesc2 rtImageDesc;
    rtImageDesc.Usage = TextureUsage::Storage;
    rtImageDesc.Width = 1280;
    rtImageDesc.Height = 720;
    rtImageDesc.Format = TextureFormat::RGBA16F;
    auto rtImage = device->CreateTexture(rtImageDesc);

    Columbus::Camera camera;

    camera.Perspective(80, 1280.f/720.f, 0.01, 1000);

    auto rtDescriptorSet = device->CreateDescriptorSet(rtPipeline, 0);
    device->UpdateDescriptorSet(rtDescriptorSet, 0, 0, tlas2);
    device->UpdateDescriptorSet(rtDescriptorSet, 1, 0, rtImage);

    auto rtIndicesSet = device->CreateDescriptorSet(rtPipeline, 1);
    auto rtUvsSet = device->CreateDescriptorSet(rtPipeline, 2);
    auto rtNormalsSet = device->CreateDescriptorSet(rtPipeline, 3);
    auto rtTexturesSet = device->CreateDescriptorSet(rtPipeline, 4);
    auto rtMaterialsSet = device->CreateDescriptorSet(rtPipeline, 5);

    for (int i = 0; i < meshes.size(); i++)
    {
        const auto& mesh = meshes[i];
        device->UpdateDescriptorSet(rtIndicesSet, 0, i, mesh.IndexBuffer);
        device->UpdateDescriptorSet(rtUvsSet, 0, i, mesh.UvBuffer);
        device->UpdateDescriptorSet(rtNormalsSet, 0, i, mesh.NormalBuffer);
        device->UpdateDescriptorSet(rtMaterialsSet, 0, i, mesh.MaterialBuffer);
    }

    for (int i = 0; i < textures.size(); i++)
    {
        device->UpdateDescriptorSet(rtTexturesSet, 0, i, textures[i]);
    }

    auto screenDescriptorSet = device->CreateDescriptorSet(screenPipeline, 0);
    device->UpdateDescriptorSet(screenDescriptorSet, 0, 0, rtImage);

    io.DisplaySize = {1280,720};

    Columbus::Timer timer;

    Vector3 lightDirection(1,1,1);
    float lightSpread = 1;

    camera.Pos = { -1173, 602, 104 };
    camera.Rot = { 0, -70, 0 };

    size_t currentFrame = 0;
    size_t frame = 0;
    bool running = true;
    bool pauseRays = false;
    bool reset = false;

    while (running)
    {
        float DeltaTime = timer.Elapsed();
        timer.Reset();

        if (!pauseRays) frame++;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        auto keyboard = SDL_GetKeyboardState(NULL);
        if (keyboard[SDL_SCANCODE_DOWN]) camera.Rot += Columbus::Vector3(5,0,0) * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_UP]) camera.Rot += Columbus::Vector3(-5,0,0) * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_LEFT]) camera.Rot += Columbus::Vector3(0,5,0) * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_RIGHT]) camera.Rot += Columbus::Vector3(0,-5,0) * DeltaTime * 20;

        if (keyboard[SDL_SCANCODE_W]) camera.Pos += camera.Direction() * DeltaTime * 50;
        if (keyboard[SDL_SCANCODE_S]) camera.Pos -= camera.Direction() * DeltaTime * 50;
        if (keyboard[SDL_SCANCODE_D]) camera.Pos += camera.Right() * DeltaTime * 50;
        if (keyboard[SDL_SCANCODE_A]) camera.Pos -= camera.Right() * DeltaTime * 50;
        if (keyboard[SDL_SCANCODE_LSHIFT]) camera.Pos += camera.Up() * DeltaTime * 50;
        if (keyboard[SDL_SCANCODE_LCTRL]) camera.Pos -= camera.Up() * DeltaTime * 50;

        camera.Update();

        uint32_t imageIndex;

        device->WaitForFence(fence, UINT64_MAX);
        device->ResetFence(fence);
        device->AcqureNextImage(swapchain, imageSemaphore, imageIndex);

        ImGui::NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        auto camDir = camera.Direction();

        // ImGui::ShowDemoWindow();
        ImGui::Begin("Properties");
            ImGui::SliderFloat3("Light direction", (float*)&lightDirection, -1, 1, "%.1f");
            ImGui::SliderFloat("Light spread", &lightSpread, 0, 1);
            ImGui::InputFloat3("Camera position", (float*)&camera.Pos);
            ImGui::InputFloat3("Camera direction", (float*)&camDir);
            ImGui::Checkbox("Pause rays", &pauseRays);
            ImGui::Checkbox("Reset", &reset);
        ImGui::End();

        ImGui::Render();

        cmdBuf.Reset();
        cmdBuf.Begin();

        VkRect2D rect;
        rect.offset = {};
        rect.extent = swapchain->swapChainExtent;

        VkClearValue clearColor = {{{1, 0, 0, 1}}};

        if (reset)
        {
            frame = 1;
        }

        struct
        {
            Vector4 camPos;
            Vector4 camDir;
            Vector4 camUp;
            Vector4 camSide;
            Vector4 lightDir;
            float lightSpread;
            int frameNumber;
            int reset;
        } rayParams;
        rayParams = { {camera.Pos,0}, {camera.Direction(),0}, {camera.Up(),0}, {camera.Right(),0}, {lightDirection,0}, lightSpread, (int)frame, (int)reset };

        if (!pauseRays)
        {
            cmdBuf.BeginDebugMarker("RayTracingPass");

            cmdBuf.BindRayTracingPipeline(rtPipeline);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 0, 1, &rtDescriptorSet);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 1, 1, &rtIndicesSet);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 2, 1, &rtUvsSet);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 3, 1, &rtNormalsSet);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 4, 1, &rtTexturesSet);
            cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 5, 1, &rtMaterialsSet);
            cmdBuf.PushConstantsRayTracing(rtPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
            cmdBuf.TraceRays(rtPipeline, 1280, 720, 1);

            cmdBuf.EndDebugMarker();
        }

        cmdBuf.BeginDebugMarker("GUI Pass");
        cmdBuf.BeginRenderPass(renderpass, rect, swapchain->swapChainFramebuffers[imageIndex], 1, &clearColor);

        struct
        {
            uint32_t frameNumber;
        } pixelData;
        pixelData.frameNumber = frame;

        cmdBuf.BindGraphicsPipeline(screenPipeline);
        cmdBuf.BindDescriptorSetsGraphics(screenPipeline, 0, 1, &screenDescriptorSet);
        cmdBuf.PushConstantsGraphics(screenPipeline, ShaderType::Pixel, 0, sizeof(pixelData), &pixelData);
        cmdBuf.Draw(3, 1, 0, 0);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf._CmdBuf);
        cmdBuf.EndRenderPass();
        cmdBuf.EndDebugMarker();

        cmdBuf.End();

        VkSemaphore semaphores[] = { imageSemaphore };

        device->Submit(cmdBuf, fence, 1, semaphores, 1, &submitSemaphore);
        device->Present(swapchain, imageIndex, submitSemaphore);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        //draw_frame();
        //device.waitIdle();
    }

    VK_CHECK(vkQueueWaitIdle(*device->_ComputeQueue));
    VK_CHECK(vkDeviceWaitIdle(device->_Device));

    vkDestroySwapchainKHR(device->_Device, swapchain->swapChain, nullptr);
    vkDestroySurfaceKHR(instance.instance, surface, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
