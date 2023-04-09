#include "Common/Image/Image.h"
#include "Common/Model/Model.h"
#include "Core/Stacktrace.h"
#include "Core/Timer.h"
#include "Graphics/Buffer.h"
#include "Graphics/Camera.h"
#include "Graphics/Device.h"
#include "Graphics/GraphicsPipeline.h"
#include "Graphics/RayTracingPipeline.h"
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
#include "Lib/imgui/examples/imgui_impl_vulkan.h"
#include "examples/imgui_impl_sdl.h"
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

std::string shaderSrc = R"(#version 450

#ifdef VERTEX_SHADER
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragWorldPos;

layout(push_constant) uniform constants
{
    mat4 m;
    mat4 mvp;
} PushConstants;

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = PushConstants.mvp * vec4(pos, 1.0);
    fragColor = colors[gl_VertexIndex];
    fragUv = uv;
    fragNormal = (PushConstants.m * vec4(normal, 1)).xyz;
    fragWorldPos = (PushConstants.m * vec4(pos, 1.0)).rgb;
}
#endif

#ifdef PIXEL_SHADER
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform sampler2D texSampler;

void main() {
    vec3 light = vec3(0, 0, 5);
    vec3 L = normalize(light - fragWorldPos);
    float ndotl = max(dot(fragNormal, L), 0);

    outColor = texture(texSampler, fragUv) * ndotl;
}
#endif
)";

std::string skyboxSrc = R"(#version 450
#ifdef VERTEX_SHADER
    layout(location = 0) in vec3 pos;

    layout(push_constant) uniform constants
    {
        mat4 viewProjection;
    } PushConstants;

    layout(location = 0) out vec3 fragPos;

    void main() {
        gl_Position = PushConstants.viewProjection * vec4(pos, 1.0);
	    fragPos = pos;
    }
#endif

#ifdef PIXEL_SHADER
    layout(location = 0) in vec3 fragPos;

    layout(location = 0) out vec4 RT0;

    layout(binding = 0, set = 0) uniform samplerCube Skybox;

    void main() {
        RT0 = texture(Skybox, fragPos).bgra;
	    gl_FragDepth = 0x7FFFFFFF;
    }
#endif
)";

std::string srcRaygen = R"(#version 460 core
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
    vec4 colorAndDist;
    vec4 normalAndObjId;
};

layout(location = 0) rayPayloadEXT RayPayload payload;
layout(location = 1) rayPayloadEXT RayPayload shadowPayload;

layout(binding = 0, set = 0) uniform accelerationStructureEXT acc;
layout(binding = 1, rgba32f) uniform image2D img;
layout(push_constant) uniform params
{
    vec4 camPos;
    vec4 camDir;
    vec4 camUp;
    vec4 camSide;
    vec4 lightDir;
    float lightSpread;
    int frameNumber;
} rayParams;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}


// Pseudo-random value in half-open range [0:1].
float rand(float x) { return floatConstruct(hash(floatBitsToUint(x))); }

// float rand(float n){return fract(sin(n) * cos(n) * 43758.5453123);}

void main() {
    vec3 camPos = rayParams.camPos.xyz;
    vec3 camDir = rayParams.camDir.xyz;
    vec3 camUp = rayParams.camUp.xyz;
    vec3 camSide = rayParams.camSide.xyz;

    const float fov = 45;
    const float aspect = float(gl_LaunchSizeEXT.x) / float(gl_LaunchSizeEXT.y);

    const vec2 uv = vec2(gl_LaunchIDEXT.xy) / vec2(gl_LaunchSizeEXT.xy - 1);

    vec3 u = camSide.xyz;
    vec3 v = camUp.xyz;

    const float planeWidth = tan(fov * 0.5f);
    u *= (planeWidth * aspect);
    v *= planeWidth;

    const vec3 rayDir = normalize(camDir.xyz + (u * (uv.x*2-1)) - (v * (uv.y*2-1)));

    const vec3 origin = camPos;
    const vec3 direction = rayDir;

    const uint cullMask = 0xFF;
    const uint sbtRecordOffset = 0;
    const uint sbtRecordStride = 0;
    const uint missIndex = 0;
    const float tmin = 0.0f;
    const float tmax = 100.0f;
    const int payloadLocation = 0;

    traceRayEXT(acc,
        gl_RayFlagsOpaqueEXT,
        cullMask,
        sbtRecordOffset,
        sbtRecordStride,
        missIndex,
        origin,
        tmin,
        direction,
        tmax,
        payloadLocation);

    vec3 hitColor = payload.colorAndDist.xyz;
    float hitDistance = payload.colorAndDist.w;
    vec3 normal = payload.normalAndObjId.xyz;
    float objId = payload.normalAndObjId.w;
    vec3 shadowOrigin = origin + direction * hitDistance;

    float lighting = 1;
    float samples = 0;
    float ranges = 0.006;
    vec3 lightDir = rayParams.lightDir.xyz;
    float ndotl = max(dot(lightDir, normal), 0);

    // if (ndotl > 0)
    // {
    //     for (float i = -ranges; i <=ranges; i += 0.01)
    //     {
    //         for (float j = -ranges; j <= ranges; j += 0.01)
    //         {
    //             vec2 rnd = fract(uv.xy) + rayParams.frameNumber % 100;
    //             // vec2 shadowDirSpherical = vec2(1,0.5); // theta, phi
    //             // shadowDirSpherical += vec2(rand(uv.x+uv.y-i*20), rand(uv.x-uv.y+j*20))/30.0;            

    //             vec3 shadowDir = normalize(vec3(
    //                 lightDir.x + rand(rnd.x+rnd.y-i*20) * rayParams.lightSpread,
    //                 lightDir.y + rand(rnd.x+rnd.y+j*20) * rayParams.lightSpread,
    //                 lightDir.z + rand(rnd.x-rnd.y-i*20) * rayParams.lightSpread
    //             ));

    //             // vec3 shadowDir = normalize(vec3(
    //             //     sin(shadowDirSpherical.y) * cos(shadowDirSpherical.x),
    //             //     sin(shadowDirSpherical.y) * sin(shadowDirSpherical.x),
    //             //     cos(shadowDirSpherical.y)
    //             // ));

    //             traceRayEXT(acc,
    //                 gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
    //                 cullMask,
    //                 sbtRecordOffset,
    //                 sbtRecordStride,
    //                 missIndex,
    //                 shadowOrigin,
    //                 0.02,
    //                 shadowDir,
    //                 tmax,
    //             1);

    //             lighting += shadowPayload.colorAndDist.w != -1 ? 1 : 0;
    //             samples += 1;
    //         }
    //     }

    //     lighting /= samples;
    // }

    if (hitDistance > 0)
    {
        traceRayEXT(acc,
            gl_RayFlagsOpaqueEXT | gl_RayFlagsTerminateOnFirstHitEXT,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            shadowOrigin,
            0.02,
            lightDir,
            tmax,
            1
        );

        if (shadowPayload.colorAndDist.w > 0) {
            lighting = 0.5;
        } else {
            lighting = max(0.5, ndotl);
        }
    }

    if (objId == 1)
    {
        vec3 reflectedDir = reflect(direction, normal);
        traceRayEXT(acc,
            gl_RayFlagsOpaqueEXT,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            shadowOrigin,
            0.02,
            reflectedDir,
            tmax,
            0);
        hitColor = payload.colorAndDist.xyz;
        lighting = 1;
    }
    

    // lighting = 1 - lighting;
    hitColor *= lighting;

    imageStore(img, ivec2(gl_LaunchIDEXT), vec4(hitColor, 1));
    // imageStore(img, ivec2(gl_LaunchIDEXT), vec4(vec3(rand(uv.x+uv.y+rayParams.frameNumber % 50)),1));
})";

std::string srcClosestHit = R"(#version 460 core
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : require

struct RayPayload {
    vec4 colorAndDist;
    vec4 normalAndObjId;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

vec2 BaryLerp(vec2 a, vec2 b, vec2 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

vec3 BaryLerp(vec3 a, vec3 b, vec3 c, vec3 barycentrics) {
    return a * barycentrics.x + b * barycentrics.y + c * barycentrics.z;
}

layout(binding = 0, set = 1) readonly buffer IndexBuffer {
    uint indices[];
} IndexBuffers[100];

layout(binding = 0, set = 2) readonly buffer UvsBuffer {
    vec2 uvs[];
} UvsBuffers[100];

layout(binding = 0, set = 3) readonly buffer NormalsBuffer {
    float normals[];
} NormalsBuffers[100];

layout(binding = 0, set = 4) uniform sampler2D Textures[100];

#define NORMALBUF NormalsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].normals

hitAttributeEXT vec2 HitAttribs;

void main() {
    vec3 barycentrics = vec3(1.0f - HitAttribs.x - HitAttribs.y, HitAttribs.x, HitAttribs.y);

    uint index0 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 0];
    uint index1 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 1];
    uint index2 = IndexBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].indices[gl_PrimitiveID * 3 + 2];

    vec2 uv0 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index0];
    vec2 uv1 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index1];
    vec2 uv2 = UvsBuffers[nonuniformEXT(gl_InstanceCustomIndexEXT)].uvs[index2];
    vec2 uv = BaryLerp(uv0, uv1, uv2, barycentrics);

    vec3 normal0 = vec3(NORMALBUF[index0*3+0], NORMALBUF[index0*3+1], NORMALBUF[index0*3+2]);
    vec3 normal1 = vec3(NORMALBUF[index1*3+0], NORMALBUF[index1*3+1], NORMALBUF[index1*3+2]);
    vec3 normal2 = vec3(NORMALBUF[index2*3+0], NORMALBUF[index2*3+1], NORMALBUF[index2*3+2]);
    vec3 normal = BaryLerp(normal0, normal1, normal2, barycentrics);

    switch (gl_InstanceCustomIndexEXT)
    {
    case 0:
        uv *= 2;
        break;
    case 1:
        break;
    case 2:
        break;
    }

    vec3 texel = textureLod(Textures[nonuniformEXT(gl_InstanceCustomIndexEXT)], uv, 0.0f).rgb;

    payload.colorAndDist = vec4(texel, gl_HitTEXT);
    payload.normalAndObjId = vec4(normal, gl_InstanceCustomIndexEXT);
})";

std::string srcRayMiss = R"(#version 460 core
#extension GL_EXT_ray_tracing : enable

struct RayPayload {
    vec4 colorAndDist;
    vec4 normalAndObjId;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

void main() {
    payload.colorAndDist = vec4(0.412f, 0.796f, 1.0f, -1);
    payload.normalAndObjId = vec4(0,0,0,0);
})";

std::string srcScreenVert = R"(#version 460 core
layout (location = 0) out vec2 texcoords;

const vec3 verts[3] = vec3[](
    vec3(-3, -1, 0),
    vec3(1, 3, 0),
    vec3(1, -1, 0)
);

layout(push_constant) uniform params
{
    vec4 asd;
    vec3 dsa;
} vertParams;

void main() {
    gl_Position = vec4(verts[gl_VertexIndex], 1);
    texcoords = 0.5 * gl_Position.xy + vec2(0.5);
}
)";

std::string srcScreenFrag = R"(#version 460 core
    layout(location = 0) in vec2 texcoords;

    layout(binding = 0, set = 0, rgba32f) uniform image2D img;

    layout(location = 0) out vec4 RT0;

    layout(push_constant) uniform fragparams
    {
        vec4 asd;
        vec3 dsa;
        mat4 qwe;
    } fragParams;

    void main() {
        RT0 = imageLoad(img, ivec2(texcoords * vec2(1280,720)));
    }
)";

const int MAX_FRAMES_IN_FLIGHT = 2;

Columbus::Vector3 triVerts[] = {
    { -1.5, -1, 0 },
    { 1, 1.5, 0 },
    { 1, -1, 0 },
};

void CrashHandler(int signal)
{
    WriteStacktraceToLog();
    exit(1);
}

int main()
{
    std::signal(SIGSEGV, CrashHandler);

    using namespace Columbus;

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

    auto buildCmdBuf = device->CreateCommandBuffer();

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

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

    Columbus::Model model;
    model.Load("Data/Meshes/Box.cmf");
    auto submodel = model.GetSubModel(0);

    std::vector<uint32_t> indices;
    for (int i = 0; i < submodel.VerticesCount; i++)
    {
        indices.push_back(i);
    }

    auto vbuf = device->CreateBuffer(submodel.VerticesCount * sizeof(Columbus::Vector3), submodel.Positions, Columbus::BufferType::Array, true, true);
    auto ubuf = device->CreateBuffer(submodel.VerticesCount * sizeof(Columbus::Vector2), submodel.UVs, Columbus::BufferType::UAV);
    auto nbuf = device->CreateBuffer(submodel.VerticesCount * sizeof(Columbus::Vector3), submodel.Normals, Columbus::BufferType::UAV);
    auto ibuf = device->CreateBuffer(sizeof(uint32_t) * indices.size(), indices.data(), Columbus::BufferType::UAV, true, true);

    Columbus::Model model2;
    model2.Load("Data/Meshes/Statue.cmf");
    auto submodel2 = model2.GetSubModel(0);

    std::vector<uint32_t> indices2;
    for (int i = 0; i < submodel2.IndicesCount; i++)
    {
        auto indicesArray = (uint16_t*)submodel2.Indices;
        indices2.push_back(indicesArray[i]);
    }

    auto vbuf2 = device->CreateBuffer(submodel2.VerticesCount * sizeof(Columbus::Vector3), submodel2.Positions, Columbus::BufferType::Array, true, true);
    auto ubuf2 = device->CreateBuffer(submodel2.VerticesCount * sizeof(Columbus::Vector2), submodel2.UVs, Columbus::BufferType::UAV);
    auto nbuf2 = device->CreateBuffer(submodel2.VerticesCount * sizeof(Columbus::Vector3), submodel2.Normals, Columbus::BufferType::UAV);
    auto ibuf2 = device->CreateBuffer(sizeof(uint32_t) * indices2.size(), indices2.data(), Columbus::BufferType::UAV, true, true);

    Columbus::Model model3;
    model3.Load("Data/Meshes/Sphere.cmf");
    auto submodel3 = model3.GetSubModel(0);

    std::vector<uint32_t> indices3;
    for (int i = 0; i < submodel3.IndicesCount; i++)
    {
        auto indicesArray = (uint16_t*)submodel3.Indices;
        indices3.push_back(indicesArray[i]);
    }

    auto vbuf3 = device->CreateBuffer(submodel3.VerticesCount * sizeof(Columbus::Vector3), submodel3.Positions, Columbus::BufferType::Array, true, true);
    auto ubuf3 = device->CreateBuffer(submodel3.VerticesCount * sizeof(Columbus::Vector2), submodel3.UVs, Columbus::BufferType::UAV);
    auto nbuf3 = device->CreateBuffer(submodel3.VerticesCount * sizeof(Columbus::Vector3), submodel3.Normals, Columbus::BufferType::UAV);
    auto ibuf3 = device->CreateBuffer(sizeof(uint32_t) * indices3.size(), indices3.data(), Columbus::BufferType::UAV, true, true);

    Columbus::AccelerationStructureDesc blasDesc;
    blasDesc.Type = Columbus::AccelerationStructureType::BLAS;
    blasDesc.Geometry = vbuf;
    blasDesc.Indices = ibuf;
    blasDesc.VerticesCount = submodel.VerticesCount;
    blasDesc.IndicesCount = indices.size();
    blasDesc.Indexed = true;
    auto blas = device->CreateAccelerationStructure(blasDesc);

    Columbus::AccelerationStructureDesc blasDesc2;
    blasDesc2.Type = Columbus::AccelerationStructureType::BLAS;
    blasDesc2.Geometry = vbuf2;
    blasDesc2.Indices = ibuf2;
    blasDesc2.VerticesCount = submodel2.VerticesCount;
    blasDesc2.IndicesCount = submodel2.IndicesCount;
    blasDesc2.Indexed = true;
    auto blas2 = device->CreateAccelerationStructure(blasDesc2);

    Columbus::AccelerationStructureDesc blasDesc3;
    blasDesc3.Type = Columbus::AccelerationStructureType::BLAS;
    blasDesc3.Geometry = vbuf3;
    blasDesc3.Indices = ibuf3;
    blasDesc3.VerticesCount = submodel3.VerticesCount;
    blasDesc3.IndicesCount = submodel3.IndicesCount;
    blasDesc3.Indexed = true;
    auto blas3 = device->CreateAccelerationStructure(blasDesc3);

    Columbus::AccelerationStructureDesc tlasDesc;
    tlasDesc.Type = Columbus::AccelerationStructureType::TLAS;
    tlasDesc.Instances = {
        { Columbus::Matrix().Scale({10,0.1,10}), blas },
        { Columbus::Matrix().Translate({0,1,0}), blas3 },
        { Columbus::Matrix().Translate({3,0,0}), blas2 }
    };
    auto tlas = device->CreateAccelerationStructure(tlasDesc);

    Columbus::Graphics::GraphicsPipelineDesc desc {};
    desc.Name = "Geometry";
    desc.layout.Elements = {
      Columbus::InputLayoutElementDesc("pos", 0, 0, 3),
      Columbus::InputLayoutElementDesc("uv", 1, 1, 2),
      Columbus::InputLayoutElementDesc("normal", 2, 2, 3),
    };
    desc.topology = Columbus::PrimitiveTopology::TriangleList;
    desc.VS = std::make_shared<Columbus::ShaderStage>(shaderSrc, "main", Columbus::ShaderType::Vertex, Columbus::ShaderLanguage::GLSL);
    desc.PS = std::make_shared<Columbus::ShaderStage>(shaderSrc, "main", Columbus::ShaderType::Pixel, Columbus::ShaderLanguage::GLSL);
	auto pipeline = device->CreateGraphicsPipeline(desc, renderpass);

    Columbus::Graphics::GraphicsPipelineDesc skyDesc {};
    skyDesc.Name = "Sky";
    skyDesc.topology = Columbus::PrimitiveTopology::TriangleList;
    skyDesc.layout.Elements = {
      Columbus::InputLayoutElementDesc("pos", 0, 0, 3)
    };
    skyDesc.rasterizerState.Cull = Columbus::CullMode::No;
    skyDesc.VS = std::make_shared<Columbus::ShaderStage>(skyboxSrc, "main", Columbus::ShaderType::Vertex, Columbus::ShaderLanguage::GLSL);
    skyDesc.PS = std::make_shared<Columbus::ShaderStage>(skyboxSrc, "main", Columbus::ShaderType::Pixel, Columbus::ShaderLanguage::GLSL);
    auto skyPipeline = device->CreateGraphicsPipeline(skyDesc, renderpass);

    Columbus::Graphics::GraphicsPipelineDesc screenDesc {};
    screenDesc.Name = "Screen";
    screenDesc.topology = Columbus::PrimitiveTopology::TriangleList;
    screenDesc.rasterizerState.Cull = Columbus::CullMode::No;
    screenDesc.VS = std::make_shared<Columbus::ShaderStage>(srcScreenVert, "main", Columbus::ShaderType::Vertex, Columbus::ShaderLanguage::GLSL);
    screenDesc.PS = std::make_shared<Columbus::ShaderStage>(srcScreenFrag, "main", Columbus::ShaderType::Pixel, Columbus::ShaderLanguage::GLSL);
    auto screenPipeline = device->CreateGraphicsPipeline(screenDesc, renderpass);

    Columbus::RayTracingPipelineDesc rtDesc{};
    rtDesc.RayGen = std::make_shared<Columbus::ShaderStage>(srcRaygen, "main", Columbus::ShaderType::Raygen, Columbus::ShaderLanguage::GLSL);
    rtDesc.Miss = std::make_shared<Columbus::ShaderStage>(srcRayMiss, "main", Columbus::ShaderType::Miss, Columbus::ShaderLanguage::GLSL);
    rtDesc.ClosestHit = std::make_shared<Columbus::ShaderStage>(srcClosestHit, "main", Columbus::ShaderType::ClosestHit, Columbus::ShaderLanguage::GLSL);
    rtDesc.MaxRecursionDepth = 1;
    auto rtPipeline = device->CreateRayTracingPipeline(rtDesc);

	Columbus::Image img, skyImg, statueImg, floorImg;
	img.Load("./Data/Textures/lantern_Albedo.png");
    skyImg.Load("./Data/Sky.dds");
    statueImg.Load("./Data/Textures/statue_d.png");
    floorImg.Load("./Data/Textures/Detail.png");

    auto skyTexture = device->CreateTexture(skyImg);
	auto texture = device->CreateTexture(img);
    auto rtImage = device->CreateStorageImage();
    auto statueImage = device->CreateTexture(statueImg);
    auto floorImage = device->CreateTexture(floorImg);

    Columbus::Camera camera;
    Columbus::Transform transform;

    camera.Perspective(80, 1280.f/720.f, 0.01, 1000);

    auto descriptorSet = device->CreateDescriptorSet(static_cast<Columbus::Graphics::GraphicsPipelineVulkan*>(pipeline)->setLayouts[0]);
    device->UpdateDescriptorSet(descriptorSet, texture);

    auto skyDescriptorSet = device->CreateDescriptorSet(static_cast<Columbus::Graphics::GraphicsPipelineVulkan*>(skyPipeline)->setLayouts[0]);
    device->UpdateDescriptorSet(skyDescriptorSet, skyTexture);

    auto rtDescriptorSet = device->CreateDescriptorSet(static_cast<Columbus::RayTracingPipelineVulkan*>(rtPipeline)->setLayouts[0]);
    device->UpdateRtDescriptorSet(rtDescriptorSet, tlas->_Handle, rtImage->view);

    auto rtIndicesSet = device->CreateDescriptorSet(static_cast<Columbus::RayTracingPipelineVulkan*>(rtPipeline)->setLayouts[1], true);
    device->UpdateArrayDescriptorSet(rtIndicesSet, 0, ibuf.Buffer);
    device->UpdateArrayDescriptorSet(rtIndicesSet, 1, ibuf3.Buffer);
    device->UpdateArrayDescriptorSet(rtIndicesSet, 2, ibuf2.Buffer);

    auto rtUvsSet = device->CreateDescriptorSet(static_cast<Columbus::RayTracingPipelineVulkan*>(rtPipeline)->setLayouts[2], true);
    device->UpdateArrayDescriptorSet(rtUvsSet, 0, ubuf.Buffer);
    device->UpdateArrayDescriptorSet(rtUvsSet, 1, ubuf3.Buffer);
    device->UpdateArrayDescriptorSet(rtUvsSet,2, ubuf2.Buffer);

    auto rtNormalsSet = device->CreateDescriptorSet(static_cast<Columbus::RayTracingPipelineVulkan*>(rtPipeline)->setLayouts[3], true);
    device->UpdateArrayDescriptorSet(rtNormalsSet, 0, nbuf.Buffer);
    device->UpdateArrayDescriptorSet(rtNormalsSet, 1, nbuf3.Buffer);
    device->UpdateArrayDescriptorSet(rtNormalsSet, 2, nbuf2.Buffer);

    auto rtTexturesSet = device->CreateDescriptorSet(static_cast<Columbus::RayTracingPipelineVulkan*>(rtPipeline)->setLayouts[4], true);
    device->UpdateArrayDescriptorSet(rtTexturesSet, 0, floorImage->view, floorImage->sampler);
    device->UpdateArrayDescriptorSet(rtTexturesSet, 1, floorImage->view, floorImage->sampler);
    device->UpdateArrayDescriptorSet(rtTexturesSet, 2, statueImage->view, statueImage->sampler);

    auto screenDescriptorSet = device->CreateDescriptorSet(static_cast<Columbus::Graphics::GraphicsPipelineVulkan*>(screenPipeline)->setLayouts[0]);
    device->UpdateDescriptorSet(screenDescriptorSet, rtImage, true);

    io.DisplaySize = {1280,720};

    Columbus::Timer timer;

    Vector3 lightDirection(1,1,1);
    float lightSpread = 1;

    size_t currentFrame = 0;
    size_t frame = 0;
    bool running = true;
    while (running)
    {
        float DeltaTime = timer.Elapsed();
        timer.Reset();
        frame++;

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

        if (keyboard[SDL_SCANCODE_W]) camera.Pos += camera.Direction() * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_S]) camera.Pos -= camera.Direction() * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_D]) camera.Pos += camera.Right() * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_A]) camera.Pos -= camera.Right() * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_LSHIFT]) camera.Pos += camera.Up() * DeltaTime * 20;
        if (keyboard[SDL_SCANCODE_LCTRL]) camera.Pos -= camera.Up() * DeltaTime * 20;

        camera.Update();

        uint32_t imageIndex;

        device->WaitForFence(fence, UINT64_MAX);
        device->ResetFence(fence);
        device->AcqureNextImage(swapchain, imageSemaphore, imageIndex);

        ImGui::NewFrame();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);

        // ImGui::ShowDemoWindow();
        ImGui::Begin("Properties");
            ImGui::SliderFloat3("Light direction", (float*)&lightDirection, -1, 1, "%.1f");
            ImGui::SliderFloat("Light spread", &lightSpread, 0, 1);
        ImGui::End();

        ImGui::Render();

        cmdBuf.Reset();
        cmdBuf.Begin();

        VkRect2D rect;
        rect.offset = {};
        rect.extent = swapchain->swapChainExtent;

        VkClearValue clearColor = {{{1, 0, 0, 1}}};

        Columbus::BufferVulkan bufs[] = { vbuf, ubuf, nbuf };
        uint64_t bufferOffsets[] = {0, 0, 0};

        transform.Rotation *= Columbus::Quaternion({1,0,1});
        transform.Scale = {1};
        transform.Update();
        auto matrix = transform.GetMatrix() * camera.GetViewProjection();

        struct
        {
            Vector4 camPos;
            Vector4 camDir;
            Vector4 camUp;
            Vector4 camSide;
            Vector4 lightDir;
            float lightSpread;
            int frameNumber;
        } rayParams;
        rayParams = { {camera.Pos,0}, {camera.Direction(),0}, {camera.Up(),0}, {camera.Right(),0}, {lightDirection,0}, lightSpread, (int)frame };

        cmdBuf.BindRayTracingPipeline(rtPipeline);
        cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 0, 1, &rtDescriptorSet);
        cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 1, 1, &rtIndicesSet);
        cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 2, 1, &rtUvsSet);
        cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 3, 1, &rtNormalsSet);
        cmdBuf.BindDescriptorSetsRayTracing(rtPipeline, 4, 1, &rtTexturesSet);
        cmdBuf.PushConstantsRayTracing(rtPipeline, ShaderType::Raygen, 0, sizeof(rayParams), &rayParams);
        cmdBuf.TraceRays(rtPipeline, 1280, 720, 1);

        cmdBuf.BeginRenderPass(renderpass, rect, swapchain->swapChainFramebuffers[imageIndex], 1, &clearColor);

        auto view = camera.GetViewMatrix();
		view.SetRow(3, Columbus::Vector4(0, 0, 0, 1));
		view.SetColumn(3, Columbus::Vector4(0, 0, 0, 1));

        // cmdBuf.BindGraphicsPipeline(skyPipeline);
        // cmdBuf.PushConstantsGraphics(skyPipeline, 0, sizeof(view), (view * camera.GetProjectionMatrix()).M);
        // cmdBuf.BindDescriptorSetsGraphics(skyPipeline, 0, 1, &skyDescriptorSet);
        // cmdBuf.BindVertexBuffers(0, 1, bufs, bufferOffsets);
        // cmdBuf.Draw(submodel.VerticesCount, 1, 0, 0);

        Columbus::Matrix matrices[] = { transform.GetMatrix(), matrix };

        // cmdBuf.BindGraphicsPipeline(pipeline);
        // cmdBuf.PushConstantsGraphics(pipeline, 0, sizeof(matrices), matrices);
        // cmdBuf.BindDescriptorSetsGraphics(pipeline, 0, 1, &descriptorSet);
        // cmdBuf.BindVertexBuffers(0, 3, bufs, bufferOffsets);
        // cmdBuf.Draw(submodel.VerticesCount, 1, 0, 0);

        cmdBuf.BindGraphicsPipeline(screenPipeline);
        cmdBuf.BindDescriptorSetsGraphics(screenPipeline, 0, 1, &screenDescriptorSet);
        cmdBuf.Draw(3, 1, 0, 0);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf._CmdBuf);
        cmdBuf.EndRenderPass();

        VkImageCopy info;

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

    device->DestroyBuffer(vbuf);
    device->DestroyBuffer(ubuf);
    device->DestroyBuffer(nbuf);

    vkDestroySwapchainKHR(device->_Device, swapchain->swapChain, nullptr);
    vkDestroySurfaceKHR(instance.instance, surface, nullptr);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
