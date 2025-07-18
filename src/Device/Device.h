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

using Microsoft::WRL::ComPtr;

constexpr uint32_t RTV_BUFFER_COUNT = 3; // Number of swap chain buffers
constexpr DXGI_FORMAT RTV_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
constexpr DXGI_FORMAT DSV_FORMAT = DXGI_FORMAT_D32_FLOAT;

class Camera;
class Renderer;

class Device {

  public:
    Device(HWND hWnd) : mHWnd(hWnd) {};
    ~Device() = default;

    bool CreateRenderer(uint32_t Width, uint32_t Height, std::unique_ptr<Renderer>& OutRenderer);

    void CreateRenderTargetView(ComPtr<ID3D12Resource>& pD3dResource,
                                D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc,
                                D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle,
                                std::unique_ptr<Resource>& OutResource);

    void CreateDepthStencilView(ComPtr<ID3D12Resource>& pD3dResource,
                                D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc,
                                D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle,
                                std::unique_ptr<Resource>& OutResource);

    bool CreateTextureResource(uint32_t Width,
                               uint32_t Height,
                               uint32_t Samples,
                               uint32_t DepthOrArraySize,
                               uint32_t NumMips,
                               DXGI_FORMAT Format,
                               D3D12_CLEAR_VALUE ClearValue,
                               UINT Flags,
                               ComPtr<ID3D12Resource>& OutResource);

  private:
    bool CreateD3D12Device(ComPtr<IDXGIFactory6>& pDxgiFactory,
                           D3D_FEATURE_LEVEL FeatureLevel,
                           bool IsHardwareDevice,
                           bool HasMaxVideoMemory,
                           ComPtr<ID3D12Device>& OutDevice) const;

    bool CreateCommandObjects(D3D12_COMMAND_LIST_TYPE Type,
                              std::unique_ptr<CommandContext>& OutContext) const;

    bool CreateSwapChain(uint32_t Width,
                         uint32_t Height,
                         uint32_t SwapChainBufferCount,
                         DXGI_FORMAT SwapChainFormat,
                         std::unique_ptr<CommandContext>& pCommandContext,
                         std::unique_ptr<SwapChain>& OutSwapChain);

    std::unique_ptr<DescriptorHeap> mRtvHeap;
    std::unique_ptr<DescriptorHeap> mDsvHeap;

    ComPtr<IDXGIFactory6> mDxgiFactory;
    ComPtr<ID3D12Device> mD3dDevice;
    ComPtr<ID3D12Debug> mD3dDebug;
    HWND mHWnd;
};

inline DXGI_FORMAT GetBaseFormat(DXGI_FORMAT defaultFormat) {
    switch (defaultFormat) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DXGI_FORMAT_R8G8B8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8A8_TYPELESS;

    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        return DXGI_FORMAT_B8G8R8X8_TYPELESS;

        // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return DXGI_FORMAT_R32G8X24_TYPELESS;

        // No Stencil
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
        return DXGI_FORMAT_R32_TYPELESS;

        // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return DXGI_FORMAT_R24G8_TYPELESS;

        // 16-bit Z w/o Stencil
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
        return DXGI_FORMAT_R16_TYPELESS;

    default:
        return defaultFormat;
    }
}

inline DXGI_FORMAT GetStencilFormat(DXGI_FORMAT defaultFormat) {
    switch (defaultFormat) {
        // 32-bit Z w/ Stencil
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
        return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

        // 24-bit Z
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}
