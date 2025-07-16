// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#include "Device.h"

#include <d3d12.h>
#include <dxgi.h>
#include <memory>

#include "Common/Debug.h"
#include "Renderer.h"

bool Device::CreateRenderer(uint32_t Width,
                            uint32_t Height,
                            std::unique_ptr<Renderer>& OutRenderer) {

#if defined(DEBUG) || defined(_DEBUG)
    // Enable the D3D12 debug layer
    if FAILED (D3D12GetDebugInterface(IID_PPV_ARGS(&mD3dDebug))) {
        DEBUGPRINT(L"Failed to get D3D12 debug interface.\n");
        return false;
    }
    mD3dDebug->EnableDebugLayer();
#endif

    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory))) {
        DEBUGPRINT(L"Failed to create DXGI factory.\n");
        return false;
    }

    if FAILED (CreateD3D12Device(mDxgiFactory, D3D_FEATURE_LEVEL_12_0, true, true, mD3dDevice)) {
        DEBUGPRINT(L"No suitable DXGI adapter and D3D12 device found.\n");
        return false;
    }

    std::unique_ptr<CommandContext> pContext;
    if (!CreateCommandObjects(D3D12_COMMAND_LIST_TYPE_DIRECT, pContext)) {
        DEBUGPRINT(L"Failed to create CommandObjects.\n");
        return false;
    }

    std::unique_ptr<SwapChain> pSwapChain;
    if (!CreateSwapChain(pSwapChain)) {
        DEBUGPRINT(L"Failed to create CommandObjects.\n");
        return false;
    }

    // All good, create a Renderer now
    OutRenderer = std::make_unique<Renderer>(std::move(pContext), std::move(pSwapChain));

    return true;
}

bool Device::CreateCommandObjects(D3D12_COMMAND_LIST_TYPE Type,
                                  std::unique_ptr<CommandContext>& OutContext) const {

    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = Type;
    QueueDesc.NodeMask = 0;

    ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if FAILED (mD3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pD3D12CommandQueue))) {
        DEBUGPRINT(L"Failed to create CommandQueue.\n");
        return false;
    }

    ComPtr<ID3D12CommandAllocator> pD3D12CommandAllocator;
    if FAILED (mD3dDevice->CreateCommandAllocator(
                   Type, IID_PPV_ARGS(pD3D12CommandAllocator.GetAddressOf()))) {
        DEBUGPRINT(L"Failed to create CommandAllocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList> pD3D12GraphicsCommandList;
    if FAILED (mD3dDevice->CreateCommandList(
                   0, Type, pD3D12CommandAllocator.Get(), nullptr,
                   IID_PPV_ARGS(pD3D12GraphicsCommandList.GetAddressOf()))) {
        DEBUGPRINT(L"Failed to create CommandList.\n");
    }

    OutContext = std::make_unique<CommandContext>(std::move(pD3D12CommandQueue),
                                                  std::move(pD3D12CommandAllocator),
                                                  std::move(pD3D12GraphicsCommandList));

    return true;
}

bool Device::CreateSwapChain(std::unique_ptr<SwapChain>& OutSwapChain) const {
    return false;
}

bool Device::CreateD3D12Device(ComPtr<IDXGIFactory6>& pDxgiFactory,
                               D3D_FEATURE_LEVEL FeatureLevel,
                               const bool IsHardwareDevice,
                               const bool HasMaxVideoMemory,
                               ComPtr<ID3D12Device>& OutDevice) const {
    SIZE_T MaxMemory = 0;

    ComPtr<IDXGIAdapter1> pAdapter;

    // Loop through all available adapters
    for (uint32_t i = 0; pDxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        // Hardware adapter
        if (IsHardwareDevice && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            pAdapter.Reset();
            continue;
        }

        // An adapter that can create a D3D12 device
        ComPtr<ID3D12Device> pD3D12Device;
        if (FAILED(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, IID_PPV_ARGS(&pD3D12Device)))) {
            pAdapter.Reset();
            continue;
        }

        // An adapter with the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            if (desc.DedicatedVideoMemory > MaxMemory) {
                MaxMemory = desc.DedicatedVideoMemory;
                OutDevice = std::move(pD3D12Device);
            }
        } else {
            OutDevice = std::move(pD3D12Device);
        }

        pAdapter.Reset();
    }

    return OutDevice != nullptr;
}
