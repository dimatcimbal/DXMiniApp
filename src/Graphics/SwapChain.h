//
// Created by dtcimbal on 16/07/2025.
#pragma once
#include <dxgi1_2.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "DescriptorHeap.h"
#include "Resource.h"

using Microsoft::WRL::ComPtr;

class SwapChain {
  public:
    SwapChain(ComPtr<ID3D12Device>& pD3dDevice, ComPtr<IDXGISwapChain1>&& pSwapChain)
        : mD3dDevice(pD3dDevice), mSwapChain1(std::move(pSwapChain)) {};
    ~SwapChain() = default;

    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);

  private:
    static constexpr uint32_t SwapChainBufferCount = 3;

    std::vector<std::unique_ptr<Resource>> mSwapChainBuffer{SwapChainBufferCount};
    ComPtr<IDXGISwapChain1> mSwapChain1;
    ComPtr<ID3D12Device>& mD3dDevice;
    std::unique_ptr<DescriptorHeap> mRtvHeap;

    DXGI_FORMAT mBackBufferFormat{DXGI_FORMAT_R8G8B8A8_UNORM};
    int mCurrBuffer = 0;
};
