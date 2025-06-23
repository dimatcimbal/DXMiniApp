// src/Graphics/DescriptorHeap
// Created by dtcimbal on 14/06/2025.
#pragma once

#include <d3d12.h>
#include <mutex>
#include <stdint.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Device;

class DescriptorHeap {
  public:
    DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                   D3D12_DESCRIPTOR_HEAP_FLAGS Flags,
                   uint32_t NumDescriptors,
                   uint32_t DescriptorSize,
                   ComPtr<ID3D12DescriptorHeap>&& pHeap)
        : mType(Type), mFlags(Flags), mNumDescriptors(NumDescriptors),
          mNumFreeDescriptors(NumDescriptors), mDescriptorSize(DescriptorSize),
          mCurrentHeap(pHeap) {};

    ~DescriptorHeap() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE Allocate(uint32_t Count = 1);

  private:
    std::mutex mAllocationMutex;

    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    D3D12_DESCRIPTOR_HEAP_FLAGS mFlags;
    uint32_t mNumDescriptors;
    uint32_t mDescriptorSize;
    uint32_t mNumFreeDescriptors;

    ComPtr<ID3D12DescriptorHeap> mCurrentHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE mCurrentHandle;
};
