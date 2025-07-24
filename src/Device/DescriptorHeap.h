//
// Created by dtcimbal on 16/07/2025.
#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <memory>

#include <wrl/client.h>

#include "Common/NonCopyable.h"
#include "Common/StackAllocatable.h"
#include "DescriptorHeap.h"

using Microsoft::WRL::ComPtr;

class DescriptorHeap {
  public:
    friend class HeapHandle;

    DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type,
                   uint32_t DescriptorSize,
                   ComPtr<ID3D12DescriptorHeap> pDescriptorHeap)
        : mType(Type), mDescriptorSize(DescriptorSize),
          mDescriptorHeap(std::move(pDescriptorHeap)) {};
    ~DescriptorHeap() = default;

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const {
        return mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    }

    uint32_t Size() const {
        return mDescriptorSize;
    }

  private:
    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    uint32_t mDescriptorSize;
    ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
};

class HeapHandle : NonCopyable, StackAllocatable {
  public:
    HeapHandle(DescriptorHeap& DescriptorHeap)
        : mDescriptorHeap(DescriptorHeap),
          mHeapHandle(DescriptorHeap.GetCPUDescriptorHandleForHeapStart()) {};
    ~HeapHandle() = default;

    CD3DX12_CPU_DESCRIPTOR_HANDLE Next() {
        CD3DX12_CPU_DESCRIPTOR_HANDLE retHandle = mHeapHandle;
        mHeapHandle.Offset(1, mDescriptorHeap.Size());
        return retHandle;
    }

  private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE mHeapHandle;
    DescriptorHeap& mDescriptorHeap;
};