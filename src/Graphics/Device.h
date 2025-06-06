// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <wrl/client.h>

#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"
#include "Display.h"

using Microsoft::WRL::ComPtr;

namespace Graphics {

class Device {

  public:
    Device(HWND hWnd) : mHWnd(hWnd) {};
    ~Device() = default;

    // Graphics init function
    bool OnCreate(uint32_t displayWidth, uint32_t displayHeight);

    bool DisplayResize(uint32_t displayWidth, uint32_t displayHeight);

    bool DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                            uint32_t NumDescriptors,
                            D3D12_DESCRIPTOR_HEAP_FLAGS Flags,
                            std::unique_ptr<DescriptorHeap>& heap);

    bool CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE QueueType,
                              std::unique_ptr<CommandQueue>& pCommandQueue);

    bool CommandListAllocate(D3D12_COMMAND_LIST_TYPE ListType,
                             std::unique_ptr<CommandList>& pCommandList);

    bool SwapChainCreate(DXGI_FORMAT SwapChainFormat,
                         DXGI_FORMAT BackBufferFormat,
                         uint32_t displayWidth,
                         uint32_t displayHeight,
                         ID3D12CommandQueue* pD3D12CommandQueue,
                         std::unique_ptr<Display>& pDisplay);

  private:
    static const uint64_t SwapChainBufferCount = 3;

    ComPtr<ID3D12Debug> mDebugController;
    ComPtr<IDXGIFactory6> mDxgiFactory;
    ComPtr<ID3D12Device> mDevice;

    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<CommandList> mCommandList;
    std::unique_ptr<DescriptorHeap> mRtvHeap;
    std::unique_ptr<DescriptorHeap> mDsvHeap;
    std::unique_ptr<Display> mDisplay;
    HWND mHWnd;
};

// Utility functions
bool FindD3D12Device(ComPtr<IDXGIFactory6>& DxgiFactory,
                     D3D_FEATURE_LEVEL FeatureLevel,
                     bool IsHardwareDevice,
                     bool HasMaxVideoMemory,
                     ComPtr<ID3D12Device>& pBestDevice);

} // namespace Graphics
