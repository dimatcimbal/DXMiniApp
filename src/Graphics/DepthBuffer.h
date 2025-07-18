//
// Created by dtcimbal on 19/07/2025.
#pragma once
#include <dxgi1_6.h>
#include <memory>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Device;
class DescriptorHeap;
class Resource;

class DepthBuffer {
  public:
    DepthBuffer(Device& pDevice, std::unique_ptr<DescriptorHeap>& pDsvHeap, DXGI_FORMAT Format)
        : mDevice(pDevice), mDsvHeap(pDsvHeap), mFormat(Format) {};
    ~DepthBuffer() = default;

    bool Resize(uint32_t NewWidth, uint32_t NewHeight);

  private:
    // Not owning resources.
    Device& mDevice;
    std::unique_ptr<DescriptorHeap>& mDsvHeap;

    // Owning resources.
    std::unique_ptr<Resource> mResource;

    DXGI_FORMAT mFormat;
};
