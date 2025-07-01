#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cmath>

#include "shader.hpp"

static constexpr float PanSpeed = 0.005f;
static constexpr float ZoomSpeed = 0.02f;
static constexpr float MaxZoom = 0.00001f;

struct
{
    float centerX{0.0f};
    float centerY{0.0f};
    uint32_t width{1};
    uint32_t height{1};
    float zoom{1.0f};
}
static state;

static SDL_Window* window;
static SDL_GPUDevice* device;
static SDL_GPUGraphicsPipeline* pipeline;

static bool Init()
{
    SDL_SetAppMetadata("Mandelbrot Set", nullptr, nullptr);
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow("Mandelbrot Set", 1000, 1000, SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }
#if SDL_PLATFORM_APPLE
    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_MSL, true, nullptr);
#else
    device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, nullptr);
#endif
    if (!device)
    {
        SDL_Log("Failed to create device: %s", SDL_GetError());
        return false;
    }
    if (!SDL_ClaimWindowForGPUDevice(device, window))
    {
        SDL_Log("Failed to create swapchain: %s", SDL_GetError());
        return false;
    }

    return true;
}

static bool CreatePipeline()
{
    SDL_GPUShader* vertShader = LoadShader(device, "render.vert");
    SDL_GPUShader* fragShader = LoadShader(device, "render.frag");
    if (!vertShader || !fragShader)
    {
        SDL_Log("Failed to load shader(s)");
        return 1;
    }

    SDL_GPUColorTargetDescription target{};
    SDL_GPUGraphicsPipelineCreateInfo info{};
    target.format = SDL_GetGPUSwapchainTextureFormat(device, window);
    info.vertex_shader = vertShader;
    info.fragment_shader = fragShader;
    info.target_info.color_target_descriptions = &target;
    info.target_info.num_color_targets = 1;
    pipeline = SDL_CreateGPUGraphicsPipeline(device, &info);
    if (!pipeline)
    {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUShader(device, vertShader);
    SDL_ReleaseGPUShader(device, fragShader);

    return true;
}

static void Draw()
{
    SDL_WaitForGPUSwapchain(device, window);
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(device);
    if (!commandBuffer)
    {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* texture;
    if (!SDL_AcquireGPUSwapchainTexture(commandBuffer, window, &texture, &state.width, &state.height))
    {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        SDL_CancelGPUCommandBuffer(commandBuffer);
        return;
    }
    if (!texture)
    {
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return;
    }

    SDL_GPUColorTargetInfo info{};
    info.texture = texture;
    info.load_op = SDL_GPU_LOADOP_CLEAR;
    info.store_op = SDL_GPU_STOREOP_STORE;
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &info, 1, nullptr);
    if (!renderPass)
    {
        SDL_Log("Failed to begin render pass: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return;
    }

    SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
    SDL_PushGPUFragmentUniformData(commandBuffer, 0, &state, sizeof(state));
    SDL_DrawGPUPrimitives(renderPass, 4, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
}

int main(int argc, char** argv)
{
    if (!Init())
    {
        SDL_Log("Failed to initialize");
        return 1;
    }
    if (!CreatePipeline())
    {
        SDL_Log("Failed to create pipeline");
        return 1;
    }

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            }
        }
        if (!running)
        {
            break;
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_W])
        {
            state.centerY = std::max(-1.0f, state.centerY - PanSpeed * state.zoom);
        }
        if (keys[SDL_SCANCODE_S])
        {
            state.centerY = std::min(1.0f, state.centerY + PanSpeed * state.zoom);
        }
        if (keys[SDL_SCANCODE_A])
        {
            state.centerX = std::max(-1.0f, state.centerX - PanSpeed * state.zoom);
        }
        if (keys[SDL_SCANCODE_D])
        {
            state.centerX = std::min(1.0f, state.centerX + PanSpeed * state.zoom);
        }
        if (keys[SDL_SCANCODE_Q])
        {
            state.zoom = std::min(1.0f, state.zoom * (1.00f + ZoomSpeed));
        }
        if (keys[SDL_SCANCODE_E])
        {
            state.zoom = std::max(MaxZoom, state.zoom * (1.0f - ZoomSpeed));
        }

        Draw();
    }

    SDL_ReleaseGPUGraphicsPipeline(device, pipeline);
    SDL_ReleaseWindowFromGPUDevice(device, window);
    SDL_DestroyGPUDevice(device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}