// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#include "Device.h"

#include <d3d12.h>
#include <dxgi.h>
#include <memory>

namespace Graphics {

bool Device::OnCreate(uint32_t displayWidth, uint32_t displayHeight) {

#if defined(DEBUG) || defined(_DEBUG)
    // Enable the D3D12 debug layer
    if FAILED (D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugController))) {
        OutputDebugString(L"Failed to get D3D12 debug interface.\n");
        return false;
    }
    mDebugController->EnableDebugLayer();
#endif

    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&mDxgiFactory))) {
        OutputDebugString(L"Failed to create DXGI factory.\n");
        return false;
    }

    if FAILED (FindD3D12Device(mDxgiFactory, D3D_FEATURE_LEVEL_12_0, true, true, mDevice)) {
        OutputDebugString(L"No suitable DXGI adapter and D3D12 device found.\n");
        return false;
    }

    // Create a graphics command queue
    if FAILED (CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandQueue)) {
        OutputDebugString(L"Failed to allocate command queue.\n");
        return false;
    }

    // Create a command list
    if FAILED (CommandListAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandList)) {
        OutputDebugString(L"Failed to allocate command list.\n");
        return false;
    }

    // Create a swap chain
    if FAILED (SwapChainCreate(DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_R10G10B10A2_UNORM,
                               displayWidth, displayHeight, mCommandQueue->GetCommandQueueRawPtr(),
                               mDisplay)) {
        OutputDebugString(L"Failed to create swap chain.\n");
        return false;
    }

    // Create RTV descriptor (the swap chain buffers)
    if FAILED (DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChainBufferCount,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, mRtvHeap)) {
        OutputDebugString(L"Failed to create rtv heap.\n");
        return false;
    }

    // Create DSV descriptor (depth buffer)
    if FAILED (DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, mDsvHeap)) {
        OutputDebugString(L"Failed to create dsv heap.\n");
        return false;
    }

    return true;
}

bool Device::CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE QueueType,
                                  std::unique_ptr<CommandQueue>& pCommandQueue) {
    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = QueueType;
    QueueDesc.NodeMask = 0;

    ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if FAILED (mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pD3D12CommandQueue))) {
        OutputDebugString(L"Failed to create CommandQueue.\n");
        return false;
    }

    ComPtr<ID3D12Fence> pFence;
    if FAILED (mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence))) {
        OutputDebugString(L"Failed to create Fence.\n");
        return false;
    }
    pFence->SetName(L"CommandQueue::mFence");

    pCommandQueue =
        std::make_unique<CommandQueue>(QueueType, std::move(pD3D12CommandQueue), std::move(pFence));

    return true;
}

bool Device::SwapChainCreate(DXGI_FORMAT SwapChainFormat,
                             DXGI_FORMAT BackBufferFormat,
                             uint32_t displayWidth,
                             uint32_t displayHeight,
                             ID3D12CommandQueue* pD3D12CommandQueue,
                             std::unique_ptr<Display>& pDisplay) {
    // Creates swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = displayWidth;
    swapChainDesc.Height = displayHeight;
    swapChainDesc.Format = SwapChainFormat;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = SwapChainBufferCount;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc;
    fsSwapChainDesc.Windowed = TRUE;

    ComPtr<IDXGISwapChain1> pSwapChain;
    if FAILED (mDxgiFactory->CreateSwapChainForHwnd(pD3D12CommandQueue, mHWnd, &swapChainDesc,
                                                    &fsSwapChainDesc, nullptr, &pSwapChain)) {
        OutputDebugString(L"Failed to create swap chain.\n");
        return false;
    }
    pDisplay = std::make_unique<Display>(displayWidth, displayHeight, std::move(pSwapChain));
    return true;
}

bool Device::CommandListAllocate(D3D12_COMMAND_LIST_TYPE ListType,
                                 std::unique_ptr<CommandList>& pCommandList) {
    ComPtr<ID3D12CommandAllocator> pD3D12CommandListAlloc;
    if FAILED (mDevice->CreateCommandAllocator(ListType, IID_PPV_ARGS(&pD3D12CommandListAlloc))) {
        OutputDebugString(L"Failed to create CommandAllocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList> pD3D12GraphicsCommandList;
    if FAILED (mDevice->CreateCommandList(0, ListType, pD3D12CommandListAlloc.Get(), nullptr,
                                          IID_PPV_ARGS(&pD3D12GraphicsCommandList))) {
        OutputDebugString(L"Failed to create CommandList.\n");
        return false;
    }

    pCommandList = std::make_unique<CommandList>(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                 std::move(pD3D12CommandListAlloc),
                                                 std::move(pD3D12GraphicsCommandList));
    // TODO IS mCommandList->Close() NEEDED?
    return true;
}

bool Device::DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                uint32_t NumDescriptors,
                                D3D12_DESCRIPTOR_HEAP_FLAGS Flags,
                                std::unique_ptr<DescriptorHeap>& heap) {
    D3D12_DESCRIPTOR_HEAP_DESC Desc{};
    Desc.Type = Type;
    Desc.NumDescriptors = NumDescriptors;
    Desc.Flags = Flags;
    Desc.NodeMask = 0;

    ComPtr<ID3D12DescriptorHeap> pHeap;
    if FAILED (mDevice->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(pHeap.GetAddressOf()))) {
        OutputDebugString(L"Failed to create a descriptor heap.\n");
        return false;
    }

    uint32_t DescSize = mDevice->GetDescriptorHandleIncrementSize(Type);

    D3D12_CPU_DESCRIPTOR_HANDLE Handle = pHeap->GetCPUDescriptorHandleForHeapStart();

    heap = std::make_unique<DescriptorHeap>(Type, DescSize, Handle, std::move(pHeap));
    return true;
}

bool Device::DisplayResize(uint32_t displayWidth, uint32_t displayHeight) {

    // mCommandQueue->IdleGPU();

    mDisplay->Resize(displayWidth, displayHeight);

    return true;
}

bool FindD3D12Device(ComPtr<IDXGIFactory6>& DxgiFactory,
                     D3D_FEATURE_LEVEL FeatureLevel,
                     const bool IsHardwareDevice,
                     const bool HasMaxVideoMemory,
                     ComPtr<ID3D12Device>& pBestDevice) {
    SIZE_T MaxMemory = 0;

    ComPtr<IDXGIAdapter1> pAdapter;

    // Loop through all available adapters
    for (uint32_t i = 0; DxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
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
                pBestDevice = std::move(pD3D12Device);
            }
        } else {
            pBestDevice = std::move(pD3D12Device);
        }

        pAdapter.Reset();
    }

    return pBestDevice != nullptr;
}

} // namespace Graphics