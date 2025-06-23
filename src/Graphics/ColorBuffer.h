//
// Created by dtcimbal on 23/06/2025.
//
#pragma once
#include <algorithm>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class ColorBuffer {
  public:
    ColorBuffer(ComPtr<ID3D12Resource>&& pResource, D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle)
        : mD3dResource(std::move(pResource)), mRtvHandle(RtvHandle) {
    }

    ~ColorBuffer() = default;

    void Reset();

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE mRtvHandle;
    ComPtr<ID3D12Resource> mD3dResource;
};
