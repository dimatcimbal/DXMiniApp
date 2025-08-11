//
// Created by dtcimbal on 27/07/2025.
#include "Device.h"
#include "Includes/WindowsInclude.h"

#include <Common/Debug.h>
#include <algorithm>
#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DebugLayer.h"

bool Device::Create(D3D_FEATURE_LEVEL FeatureLevel,
                    bool IsHardwareDevice,
                    bool HasMaxVideoMemory,
                    std::unique_ptr<Device>& OutDevice) {

    Microsoft::WRL::ComPtr<IDXGIFactory7> pDxgiFactory;
    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory))) {
        DEBUG_ERROR(L"Failed to create DXGI factory.\n");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device14> pBestDevice;

    SIZE_T MaxMemory{0};
    for (uint32_t i{0}; pDxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        // Hardware adapter
        if (IsHardwareDevice && (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
            pAdapter.Reset();
            continue;
        }

        // An adapter that can create a D3D12 device
        Microsoft::WRL::ComPtr<ID3D12Device14> pD3D12Device;
        if (FAILED(D3D12CreateDevice(pAdapter.Get(), FeatureLevel, IID_PPV_ARGS(&pD3D12Device)))) {
            pAdapter.Reset();
            continue;
        }

        // An adapter with the *most* dedicated video memory
        if (HasMaxVideoMemory) {
            // An adapter with the *most* dedicated video memory
            if (desc.DedicatedVideoMemory > MaxMemory) {
                MaxMemory = desc.DedicatedVideoMemory;
                pBestDevice = std::move(pD3D12Device);
            }
        } else {
            pBestDevice = std::move(pD3D12Device);
        }

        // pAdapter.Reset();
    }

    if (!pBestDevice) {
        DEBUG_ERROR("No suitable D3D12 device found.\n");
        return false; // No suitable device found
    }

    OutDevice = std::make_unique<Device>(std::move(pDxgiFactory), std::move(pBestDevice));
    return true;
}

bool Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                                    D3D12_COMMAND_LIST_FLAGS Flags,
                                    std::unique_ptr<CommandAllocator>& OutAllocator) {

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pD3D12CommandAllocator;
    if FAILED (mD3dDevice->CreateCommandAllocator(Type, IID_PPV_ARGS(&pD3D12CommandAllocator))) {
        DEBUG_ERROR(L"Failed to create ID3D12CommandAllocator.\n");
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10> pD3D12CommandList;
    if FAILED (mD3dDevice->CreateCommandList1(0, Type, Flags, IID_PPV_ARGS(&pD3D12CommandList))) {
        DEBUG_ERROR("Failed to create ID3D12GraphicsCommandList10.\n");
        return false;
    }

    OutAllocator = std::make_unique<CommandAllocator>(Type, std::move(pD3D12CommandAllocator),
                                                      std::move(pD3D12CommandList));
    return true;
}

bool Device::CreateCommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_QUEUE_PRIORITY Priority,
                                D3D12_COMMAND_QUEUE_FLAGS Flags,
                                std::unique_ptr<CommandQueue>& OutQueue) {
    // The command queue desc
    D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
    QueueDesc.Type = Type;
    QueueDesc.Priority = Priority;
    QueueDesc.NodeMask = 0; // default node mask
    QueueDesc.Flags = Flags;

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> pD3D12CommandQueue;
    if FAILED (mD3dDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pD3D12CommandQueue))) {
        DEBUG_ERROR(L"Failed to create ID3D12CommandQueue.\n");
        return false;
    }

    uint32_t InitFenceValue = 0; // initial fence value
    Microsoft::WRL::ComPtr<ID3D12Fence1> pD3D12Fence;
    if FAILED (mD3dDevice->CreateFence(InitFenceValue, D3D12_FENCE_FLAG_NONE,
                                       IID_PPV_ARGS(&pD3D12Fence))) {
        DEBUG_ERROR(L"Failed to create ID3D12Fence1.\n");
        return false;
    }

    // Create an event handle for queue synchronization
    HANDLE EventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (EventHandle == nullptr) {
        DEBUG_ERROR(L"Failed to create event handle.\n");
        return false;
    }

    OutQueue = std::make_unique<CommandQueue>(
        Type, InitFenceValue, EventHandle, std::move(pD3D12CommandQueue), std::move(pD3D12Fence));
    return true;
}
