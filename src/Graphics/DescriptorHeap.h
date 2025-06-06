// src/Graphics/DescriptorHeap
// Created by dtcimbal on 14/06/2025.
#pragma once
#include <algorithm>
#include <d3d12.h>
#include <stdint.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class DescriptorHeap {
  public:
    DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                   uint32_t DescriptorSize,
                   D3D12_CPU_DESCRIPTOR_HANDLE Handle,
                   ComPtr<ID3D12DescriptorHeap>&& DescHeap)
        : mType(Type), mDescriptorSize(DescriptorSize), mHandle(Handle),
          mDescHeap(std::move(DescHeap)) {};
    ~DescriptorHeap() = default;

  private:
    ComPtr<ID3D12DescriptorHeap> mDescHeap;
    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    D3D12_CPU_DESCRIPTOR_HANDLE mHandle;
    uint32_t mDescriptorSize;
};
