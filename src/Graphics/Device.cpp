// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#include "Device.h"

#include <d3d12.h>
#include <dxgi.h>
#include <memory>

#include "../Util/Debug.h"
#include "ColorBuffer.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"

bool Device::OnCreate(uint32_t displayWidth, uint32_t displayHeight) {

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

    if FAILED (FindD3D12Device(mDxgiFactory, D3D_FEATURE_LEVEL_12_0, true, true, mD3dDevice)) {
        DEBUGPRINT(L"No suitable DXGI adapter and D3D12 device found.\n");
        return false;
    }

    // Create a graphics command queue
    if FAILED (CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandQueue)) {
        DEBUGPRINT(L"Failed to allocate command queue.\n");
        return false;
    }

    // Create a command list
    if FAILED (CommandListAllocate(D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandList)) {
        DEBUGPRINT(L"Failed to allocate command list.\n");
        return false;
    }

    // Create a swap chain
    if FAILED (SwapChainCreate(displayWidth, displayHeight, mCommandQueue, mSwapChain)) {
        DEBUGPRINT(L"Failed to create swap chain.\n");
        return false;
    }

    // Create RTV heap (the swap chain buffers)
    if FAILED (DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, SwapChainBufferCount,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, mRtvHeap)) {
        DEBUGPRINT(L"Failed to create rtv heap.\n");
        return false;
    }

    // Create DSV heap (depth buffer)
    if FAILED (DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1,
                                  D3D12_DESCRIPTOR_HEAP_FLAG_NONE, mDsvHeap)) {
        DEBUGPRINT(L"Failed to create dsv heap.\n");
        return false;
    }

    // TODO SRV descriptors (shader resource view)

    return true;
}

bool Device::CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE QueueType,
                                  std::unique_ptr<CommandQueue>& pCommandQueue) {
    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = QueueType;
    QueueDesc.NodeMask = 0;

    ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if FAILED (mD3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pD3D12CommandQueue))) {
        DEBUGPRINT(L"Failed to create CommandQueue.\n");
        return false;
    }

    ComPtr<ID3D12Fence> pFence;
    if FAILED (mD3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence))) {
        DEBUGPRINT(L"Failed to create Fence.\n");
        return false;
    }
    pFence->SetName(L"CommandQueue::mFence");

    pCommandQueue =
        std::make_unique<CommandQueue>(QueueType, std::move(pD3D12CommandQueue), std::move(pFence));

    return true;
}

bool Device::SwapChainCreate(uint32_t DisplayWidth,
                             uint32_t DisplayHeight,
                             const std::unique_ptr<CommandQueue>& pCommandQueue,
                             ComPtr<IDXGISwapChain1>& pSwapChain) {
    // Creates swap chain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = DisplayWidth;
    swapChainDesc.Height = DisplayHeight;
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

    if FAILED (mDxgiFactory->CreateSwapChainForHwnd(pCommandQueue->Get(), mHWnd, &swapChainDesc,
                                                    &fsSwapChainDesc, nullptr, &pSwapChain)) {
        DEBUGPRINT(L"Failed to create swap chain.\n");
        return false;
    }

    return true;
}

bool Device::CommandListAllocate(D3D12_COMMAND_LIST_TYPE ListType,
                                 std::unique_ptr<CommandList>& pCommandList) {
    ComPtr<ID3D12CommandAllocator> pD3D12CommandListAlloc;
    if FAILED (mD3dDevice->CreateCommandAllocator(ListType,
                                                  IID_PPV_ARGS(&pD3D12CommandListAlloc))) {
        DEBUGPRINT(L"Failed to create CommandAllocator.\n");
        return false;
    }

    ComPtr<ID3D12GraphicsCommandList> pD3D12GraphicsCommandList;
    if FAILED (mD3dDevice->CreateCommandList(0, ListType, pD3D12CommandListAlloc.Get(), nullptr,
                                             IID_PPV_ARGS(&pD3D12GraphicsCommandList))) {
        DEBUGPRINT(L"Failed to create CommandList.\n");
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
    uint32_t DescSize = mD3dDevice->GetDescriptorHandleIncrementSize(Type);

    D3D12_DESCRIPTOR_HEAP_DESC Desc{};
    Desc.Type = Type;
    Desc.NumDescriptors = NumDescriptors;
    Desc.Flags = Flags;
    Desc.NodeMask = 0;

    // Creates descriptorHeap
    ComPtr<ID3D12DescriptorHeap> pHeap;
    if FAILED (mD3dDevice->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(pHeap.GetAddressOf()))) {
        DEBUGPRINT(L"Failed to create a descriptor heap.\n");
        return false;
    }

    heap =
        std::make_unique<DescriptorHeap>(Type, Flags, NumDescriptors, DescSize, std::move(pHeap));

    return true;
}

bool Device::DisplayResize(uint32_t DisplayWidth, uint32_t DisplayHeight) {
    // Advance the fence value to mark commands up to this fence point and wait for their completion
    mCommandQueue->IncrementFence();
    mCommandQueue->WaitForIdle();

    mWidth = DisplayWidth;
    mHeight = DisplayHeight;

    DEBUGPRINT(L"Changing display resolution to %ux%u", mWidth, mHeight);

    if FAILED (mCommandList->Reset()) {
        DEBUGPRINT(L"Failed to reset CommandList.");
        return false;
    }

    for (uint32_t i = 0; i < SwapChainBufferCount; ++i) {
        if (mSwapChainBuffer[i]) {
            mSwapChainBuffer[i]->Reset();
        }
    }

    if (mDepthStencilBuffer) {
        mDepthStencilBuffer->Reset();
    }

    if FAILED (mSwapChain->ResizeBuffers(SwapChainBufferCount, mWidth, mHeight, SwapChainFormat,
                                         DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)) {
        DEBUGPRINT(L"Failed to resize swap chain buffers.");
        return false;
    }

    mCurrentBuffer = 0;

    // For each SwapChain's buffer
    for (uint32_t i = 0; i < SwapChainBufferCount; ++i) {
        ComPtr<ID3D12Resource> DisplayPlane;
        if FAILED (mSwapChain->GetBuffer(i, IID_PPV_ARGS(&DisplayPlane))) {
            DEBUGPRINT(L"Failed to get swap chain buffer %u.", i);
            return false;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{mRtvHeap->Allocate()};
        mD3dDevice->CreateRenderTargetView(DisplayPlane.Get(), nullptr, rtvHandle);
        mSwapChainBuffer[i] = std::make_unique<ColorBuffer>(std::move(DisplayPlane), rtvHandle);
    }

    return true;
}

bool FindD3D12Device(ComPtr<IDXGIFactory6>& pDxgiFactory,
                     D3D_FEATURE_LEVEL FeatureLevel,
                     const bool IsHardwareDevice,
                     const bool HasMaxVideoMemory,
                     ComPtr<ID3D12Device>& pBestDevice) {
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
                pBestDevice = std::move(pD3D12Device);
            }
        } else {
            pBestDevice = std::move(pD3D12Device);
        }

        pAdapter.Reset();
    }

    return pBestDevice != nullptr;
}
