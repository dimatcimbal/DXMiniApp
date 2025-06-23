// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "ColorBuffer.h"
#include "CommandList.h"
#include "CommandQueue.h"
#include "DescriptorHeap.h"

using Microsoft::WRL::ComPtr;

class Device {

  public:
    Device(HWND hWnd) : mHWnd(hWnd) {};
    ~Device() = default;

    // Graphics init function
    bool OnCreate(uint32_t displayWidth, uint32_t displayHeight);

    bool DisplayResize(uint32_t DisplayWidth, uint32_t DisplayHeight);

    bool DescriptorAllocate(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                            uint32_t NumDescriptors,
                            D3D12_DESCRIPTOR_HEAP_FLAGS Flags,
                            std::unique_ptr<DescriptorHeap>& heap);

    bool CommandQueueAllocate(D3D12_COMMAND_LIST_TYPE QueueType,
                              std::unique_ptr<CommandQueue>& pCommandQueue);

    bool CommandListAllocate(D3D12_COMMAND_LIST_TYPE ListType,
                             std::unique_ptr<CommandList>& pCommandList);

    bool SwapChainCreate(uint32_t displayWidth,
                         uint32_t displayHeight,
                         const std::unique_ptr<CommandQueue>& pCommandQueue,
                         ComPtr<IDXGISwapChain1>& pSwapChain);

    ID3D12Device* Get() const {
        return mD3dDevice.Get();
    }

  private:
    static constexpr uint32_t SwapChainBufferCount = 3;
    static constexpr DXGI_FORMAT SwapChainFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
    static constexpr DXGI_FORMAT BackBufferFormat = DXGI_FORMAT_R10G10B10A2_UNORM;

    ComPtr<IDXGISwapChain1> mSwapChain;
    std::vector<std::unique_ptr<ColorBuffer>> mSwapChainBuffer{SwapChainBufferCount};
    std::unique_ptr<ColorBuffer> mDepthStencilBuffer;
    UINT mCurrentBuffer{0};

    ComPtr<ID3D12Debug> mD3dDebug;
    ComPtr<IDXGIFactory6> mDxgiFactory;
    ComPtr<ID3D12Device> mD3dDevice;

    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<CommandList> mCommandList;
    std::unique_ptr<DescriptorHeap> mRtvHeap;
    std::unique_ptr<DescriptorHeap> mDsvHeap;
    HWND mHWnd;

    uint32_t mWidth{0};
    uint32_t mHeight{0};
};

// Utility functions
bool FindD3D12Device(ComPtr<IDXGIFactory6>& pDxgiFactory,
                     D3D_FEATURE_LEVEL FeatureLevel,
                     bool IsHardwareDevice,
                     bool HasMaxVideoMemory,
                     ComPtr<ID3D12Device>& pBestDevice);
