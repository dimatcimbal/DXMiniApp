//
// Created by dtcimbal on 16/07/2025.
#pragma once
#include <dxgi1_2.h>
#include <memory>
#include <vector>
#include <wrl/client.h>

#include "Device/DescriptorHeap.h"
#include "Resources/Resource.h"

using Microsoft::WRL::ComPtr;

class Device;

class SwapChain {
  public:
    SwapChain(Device& pDevice,
              std::unique_ptr<DescriptorHeap>& pRtvHeap,
              uint32_t SwapChainBufferCount,
              DXGI_FORMAT Format,
              ComPtr<IDXGISwapChain1>&& pSwapChain)
        : mDevice(pDevice), mRtvHeap(pRtvHeap), mSwapChainBufferCount(SwapChainBufferCount),
          mSwapChainBuffers{mSwapChainBufferCount}, mFormat(Format),
          mDxgiSwapChain1(std::move(pSwapChain)) {};
    ~SwapChain() = default;

    bool Resize(uint32_t NewWidth, uint32_t NewHeight);

  private:
    // Not owning resources.
    Device& mDevice;
    std::unique_ptr<DescriptorHeap>& mRtvHeap;

    // Owning resources.
    uint32_t mSwapChainBufferCount;
    std::vector<std::unique_ptr<Resource>> mSwapChainBuffers;
    ComPtr<IDXGISwapChain1> mDxgiSwapChain1;

    DXGI_FORMAT mFormat;
    int mCurrBuffer = 0;
};
