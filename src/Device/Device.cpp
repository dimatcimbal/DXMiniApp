// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#include "Device.h"

#include <d3d12.h>
#include <dxgi.h>
#include <memory>

#include "Common/Debug.h"
#include "Graphics/Color.h"
#include "Graphics/DepthBuffer.h"
#include "Graphics/Renderer.h"

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

    // Create the DXGI factory
    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory))) {
        DEBUGPRINT(L"Failed to create DXGI factory.\n");
        return false;
    }

    // Create the D3D12 device
    if FAILED (CreateD3D12Device(mDxgiFactory, D3D_FEATURE_LEVEL_12_0, true, true, mD3dDevice)) {
        DEBUGPRINT(L"No suitable DXGI adapter and D3D12 device found.\n");
        return false;
    }

    // CommandContext creation
    std::unique_ptr<CommandContext> pContext;
    if (!CreateCommandObjects(D3D12_COMMAND_LIST_TYPE_DIRECT, pContext)) {
        DEBUGPRINT(L"Failed to create CommandObjects.\n");
        return false;
    }

    // SwapChain creation
    std::unique_ptr<SwapChain> pSwapChain;
    if (!CreateSwapChain(Width, Height, RTV_BUFFER_COUNT, RTV_FORMAT, pContext, pSwapChain)) {
        DEBUGPRINT(L"Failed to create CommandObjects.\n");
        return false;
    }

    // DepthBuffer creation
    std::unique_ptr<DepthBuffer> pDepthBuffer =
        std::make_unique<DepthBuffer>(*this, mDsvHeap, DSV_FORMAT);

    // All good, create a Renderer now
    OutRenderer = std::make_unique<Renderer>(std::move(pContext), std::move(pSwapChain),
                                             std::move(pDepthBuffer));

    if (!OutRenderer->OnResize(Width, Height)) {
        DEBUGPRINT(L"Failed to initialize Renderer.\n");
        return false;
    }

    return true;
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

bool Device::CreateSwapChain(uint32_t Width,
                             uint32_t Height,
                             uint32_t BufferCount,
                             DXGI_FORMAT Format,
                             std::unique_ptr<CommandContext>& pCommandContext,
                             std::unique_ptr<SwapChain>& OutSwapChain) {

    // Creates swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = Width;
    swapChainDesc.Height = Height;
    swapChainDesc.Format = Format;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = BufferCount;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc;
    fsSwapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain1> OutDxgiSwapChain1;
    if FAILED (mDxgiFactory->CreateSwapChainForHwnd(pCommandContext->GetCommandQueue(), mHWnd,
                                                    &swapChainDesc, &fsSwapChainDesc, nullptr,
                                                    OutDxgiSwapChain1.GetAddressOf())) {
        DEBUGPRINT(L"Failed to create swap chain.\n");
        return false;
    }

    OutSwapChain = std::make_unique<SwapChain>(*this, mRtvHeap, BufferCount, RTV_FORMAT,
                                               std::move(OutDxgiSwapChain1));

    return true;
}

void Device::CreateRenderTargetView(ComPtr<ID3D12Resource>& pD3dResource,
                                    D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc,
                                    D3D12_CPU_DESCRIPTOR_HANDLE& rtvHandle,
                                    std::unique_ptr<Resource>& OutResource) {
    mD3dDevice->CreateRenderTargetView(pD3dResource.Get(), rtvDesc, rtvHandle);
    OutResource = std::make_unique<Resource>(rtvHandle, std::move(pD3dResource));
}

void Device::CreateDepthStencilView(ComPtr<ID3D12Resource>& pD3dResource,
                                    D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc,
                                    D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle,
                                    std::unique_ptr<Resource>& OutResource) {
    mD3dDevice->CreateDepthStencilView(pD3dResource.Get(), dsvDesc, dsvHandle);
    OutResource = std::make_unique<Resource>(dsvHandle, std::move(pD3dResource));
}

// template <typename T, typename = EnableIfResourceDerived<T>>
bool Device::CreateTextureResource(uint32_t Width,
                                   uint32_t Height,
                                   uint32_t Samples,
                                   uint32_t DepthOrArraySize,
                                   uint32_t NumMips,
                                   DXGI_FORMAT Format,
                                   D3D12_CLEAR_VALUE ClearValue,
                                   UINT Flags,
                                   ComPtr<ID3D12Resource>& OutResource) {

    D3D12_RESOURCE_DESC Desc = {};
    Desc.Alignment = 0;
    Desc.DepthOrArraySize = static_cast<UINT16>(DepthOrArraySize);
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    Desc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(Flags);
    Desc.Format = GetBaseFormat(Format);
    Desc.Height = Height;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    Desc.Width = Width;
    Desc.MipLevels = static_cast<UINT16>(NumMips);
    Desc.SampleDesc.Count = Samples;
    Desc.SampleDesc.Quality = 0;
    Desc.Width = static_cast<UINT16>(Width);

    CD3DX12_HEAP_PROPERTIES HeapProps(D3D12_HEAP_TYPE_DEFAULT);
    if FAILED (mD3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &Desc,
                                                   D3D12_RESOURCE_STATE_COMMON, &ClearValue,
                                                   IID_PPV_ARGS(OutResource.GetAddressOf()))) {
        DEBUGPRINT(L"Failed to create texture resource.\n");
        return false;
    }

    return true;
}
