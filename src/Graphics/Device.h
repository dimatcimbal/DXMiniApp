// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <wrl/client.h>
#include "CommandObjects/CommandContext.h"
#include "Graphics/SwapChain.h"

class Camera;
class Renderer;

using Microsoft::WRL::ComPtr;

class Device {

  public:
    Device(HWND hWnd) : mHWnd(hWnd) {};
    ~Device() = default;

    bool CreateRenderer(uint32_t Width, uint32_t Height, std::unique_ptr<Renderer>& OutRenderer);

  private:
    bool CreateSwapChain(std::unique_ptr<SwapChain>& OutSwapChain) const;

    bool CreateCommandObjects(D3D12_COMMAND_LIST_TYPE Type,
                              std::unique_ptr<CommandContext>& OutContext) const;

    bool CreateD3D12Device(ComPtr<IDXGIFactory6>& pDxgiFactory,
                           D3D_FEATURE_LEVEL FeatureLevel,
                           bool IsHardwareDevice,
                           bool HasMaxVideoMemory,
                           ComPtr<ID3D12Device>& OutDevice) const;

    ComPtr<IDXGIFactory6> mDxgiFactory;
    ComPtr<ID3D12Device> mD3dDevice;
    ComPtr<ID3D12Debug> mD3dDebug;
    HWND mHWnd;
};
