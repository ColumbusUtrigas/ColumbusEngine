#include "Main.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define AMD_EXTENSIONS
#define NV_EXTENSIONS
#include <shaderc/shaderc.hpp>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_vulkan.h>

#include <vector>
#include <algorithm>

std::string shaderSrc = R"(#version 450

#ifdef VERTEX_SHADER
layout(location = 0) out vec3 fragColor;

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
}
#endif

#ifdef PIXEL_SHADER
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
#endif
)";

SDL_Window* window;

const int MAX_FRAMES_IN_FLIGHT = 2;

vk::SurfaceKHR surface;
vk::Instance instance;
vk::PhysicalDevice physicalDevice;
vk::Device device;
vk::Queue graphicsQueue;
vk::RenderPass renderPass;
vk::CommandPool cmdPool;
std::vector<vk::CommandBuffer> cmdBuffers;
vk::Pipeline pipeline;

VkSwapchainKHR swapChain;
std::vector<VkImage> swapChainImages;
VkFormat swapChainImageFormat;
VkExtent2D swapChainExtent;
std::vector<VkImageView> swapChainImageViews;
std::vector<VkFramebuffer> swapChainFramebuffers;

std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkFence> imagesInFlight;
size_t currentFrame = 0;

uint32_t queueId;

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    return capabilities.currentExtent;

    /*if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }*/
}

void create_window()
{
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Vulkan", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_VULKAN);
}

void create_instance()
{
    std::vector<const char*> layers;
    std::vector<const char*> extensions{
        "VK_KHR_surface",
        "VK_KHR_win32_surface"
    };

    vk::ApplicationInfo applicationInfo("Test", 1, "Columbus Engine", 1, VK_API_VERSION_1_1);
    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo, layers, extensions);
    instance = vk::createInstance(instanceCreateInfo);
}

void create_surface()
{
    VkSurfaceKHR c_surface;
    SDL_Vulkan_CreateSurface(window, static_cast<VkInstance>(instance), &c_surface);
    surface = c_surface;
}

void create_device()
{
    std::vector<const char*> extensions{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // get first available physical device
    physicalDevice = instance.enumeratePhysicalDevices().front();
    auto props = physicalDevice.getProperties();
    auto features = physicalDevice.getFeatures();
    auto memoryProps = physicalDevice.getMemoryProperties();
    auto queueFamilies = physicalDevice.getQueueFamilyProperties();

    // find first suitable queue family
    auto queueIt = std::find_if(begin(queueFamilies), end(queueFamilies), [](vk::QueueFamilyProperties it) {
        return (VkFlags)it.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        });
    queueId = (uint32_t)std::distance(begin(queueFamilies), queueIt);

    std::vector<vk::DeviceQueueCreateInfo> queues{
        { {}, queueId, 1 }
    };
    vk::DeviceCreateInfo deviceInfo({}, queues, {}, extensions);
    device = physicalDevice.createDevice(deviceInfo);
    graphicsQueue = device.getQueue(queueId, 0);
}

void create_swapchain()
{
    auto swapChainSupport = querySwapChainSupport(physicalDevice);
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    /*auto indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else */{
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void create_image_views()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void create_render_pass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    renderPass = device.createRenderPass(renderPassInfo);
}

void create_graphics_pipeline()
{
    shaderc::Compiler compiler;
    shaderc::CompileOptions vs_options, ps_options;

    vs_options.AddMacroDefinition("VERTEX_SHADER");
    ps_options.AddMacroDefinition("PIXEL_SHADER");

    auto vs = compiler.CompileGlslToSpv(shaderSrc, shaderc_vertex_shader, "name", vs_options);
    auto ps = compiler.CompileGlslToSpv(shaderSrc, shaderc_fragment_shader, "name", ps_options);

    auto vsm = device.createShaderModuleUnique(vk::ShaderModuleCreateInfo{ {}, (size_t)(vs.end() - vs.begin()) * 4, vs.begin() });
    auto psm = device.createShaderModuleUnique(vk::ShaderModuleCreateInfo{ {}, (size_t)(ps.end() - ps.begin()) * 4, ps.begin() });

    vk::PipelineShaderStageCreateInfo vsinfo({}, vk::ShaderStageFlagBits::eVertex, *vsm, "main");
    vk::PipelineShaderStageCreateInfo psinfo({}, vk::ShaderStageFlagBits::eFragment, *psm, "main");
    auto stages = { vsinfo, psinfo };

    auto layout = device.createPipelineLayout(vk::PipelineLayoutCreateInfo{});

    auto colorFlags = (vk::ColorComponentFlagBits)vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags;

    std::vector<vk::Viewport> viewports{ { 0, 0, (float)swapChainExtent.width, (float)swapChainExtent.height, 0, 1 } };
    std::vector<vk::Rect2D> scissors{ { {}, swapChainExtent } };
    std::vector<vk::PipelineColorBlendAttachmentState> attachments{
        vk::PipelineColorBlendAttachmentState(false).setColorWriteMask(colorFlags)
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer({}, false, false, vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise);
    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssembly({}, vk::PrimitiveTopology::eTriangleList, false);
    vk::PipelineViewportStateCreateInfo viewportState({}, viewports, scissors);
    vk::PipelineMultisampleStateCreateInfo multisampling{};
    vk::PipelineColorBlendStateCreateInfo colorBlendState({}, false, vk::LogicOp::eCopy, attachments, { 0, 0, 0, 0 });

    vk::GraphicsPipelineCreateInfo info;
    info.setStages(stages);
    info.setPVertexInputState(&vertexInputInfo);
    info.setPInputAssemblyState(&inputAssembly);
    info.setPViewportState(&viewportState);
    info.setPRasterizationState(&rasterizer);
    info.setPMultisampleState(&multisampling);
    info.setPColorBlendState(&colorBlendState);
    info.setLayout(layout);
    info.setRenderPass(renderPass);
    info.setSubpass(0);
    info.setBasePipelineHandle(nullptr);

    pipeline = device.createGraphicsPipeline(nullptr, info).value;
}

void create_framebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void create_command_pool()
{
    cmdPool = device.createCommandPool(vk::CommandPoolCreateInfo{ {}, queueId });
}

void create_command_buffers()
{
    cmdBuffers.resize(swapChainFramebuffers.size());
    cmdBuffers = device.allocateCommandBuffers({ cmdPool, vk::CommandBufferLevel::ePrimary, (uint32_t)cmdBuffers.size()});

    for (size_t i = 0; i < cmdBuffers.size(); i++)
    {
        auto &cmdBuffer = cmdBuffers[i];
        cmdBuffer.begin(vk::CommandBufferBeginInfo{});

        std::vector<vk::ClearValue> clearValues{
            { vk::ClearColorValue().setFloat32({ 1, 0, 0, 1 }) }
        };

        cmdBuffer.beginRenderPass(
            vk::RenderPassBeginInfo{
                renderPass, swapChainFramebuffers[i], vk::Rect2D{{}, swapChainExtent}, clearValues
            }, vk::SubpassContents::eInline
        );

        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
        cmdBuffer.draw(3, 1, 0, 0);

        cmdBuffer.endRenderPass();

        cmdBuffer.end();
    }
}

void create_sync_objects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void draw_frame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    auto buf = (VkCommandBuffer)cmdBuffers[imageIndex];

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buf;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(graphicsQueue, &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

int main()
{
    create_window();
    create_instance();
    create_surface();
    create_device();
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_pool();
    create_command_buffers();
    create_sync_objects();

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        draw_frame();
        device.waitIdle();
    }

    instance.destroySurfaceKHR(surface);
    SDL_DestroyWindow(window);
    SDL_Quit();
    instance.destroy();

    return 0;
}
