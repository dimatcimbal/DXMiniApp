//
// Created by dtcimbal on 15/07/2025.
#pragma once

#include <d3d12.h>
#include <memory>
#include "DepthBuffer.h"

class Camera;
class CommandContext;
class Resource;
class SwapChain;

class Renderer {
  public:
    Renderer(std::unique_ptr<CommandContext>&& pCommandContext,
             std::unique_ptr<SwapChain>&& pSwapChain,
             std::unique_ptr<DepthBuffer>&& pDepthBuffer)
        : mCommandContext(std::move(pCommandContext)), mSwapChain(std::move(pSwapChain)),
          mDepthBuffer(std::move(pDepthBuffer)) {};
    Renderer() = default;

    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);
    bool Draw(const std::unique_ptr<Camera>& pCamera);

  private:
    // Owning resources.
    std::unique_ptr<CommandContext> mCommandContext;
    std::unique_ptr<SwapChain> mSwapChain;
    std::unique_ptr<DepthBuffer> mDepthBuffer;

    D3D12_VIEWPORT mScreenViewport;
    D3D12_RECT mScissorRect;
};
