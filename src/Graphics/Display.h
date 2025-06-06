// src/Graphics/Display.h
// Created by dtcimbal on 13/06/2025.
#pragma once
#include <dxgi1_2.h>
#include <memory>

#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

/*
 * Display is the holder of DXGISwapChain.
 */
class Display {
  public:
    Display(uint32_t Width, uint32_t Height, ComPtr<IDXGISwapChain1>&& SwapChain)
        : mWidth(Width), mHeight(Height), mSwapChain(std::move(SwapChain)) {};
    bool Resize(uint32_t Width, uint32_t Height);

  private:
    ComPtr<IDXGISwapChain1> mSwapChain;
    uint32_t mWidth;
    uint32_t mHeight;
};
