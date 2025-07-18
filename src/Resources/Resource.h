//
// Created by dtcimbal on 16/07/2025.
//
#pragma once

#include <d3d12.h>
#include <d3dx12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class Resource {
  public:
    Resource(D3D12_CPU_DESCRIPTOR_HANDLE RTVHandle, ComPtr<ID3D12Resource>&& pResource)
        : mDescriptorHandle(RTVHandle), mResource(std::move(pResource)) {};
    ~Resource() = default;

    ID3D12Resource* Get() {
        return mResource.Get();
    }

  private:
    ComPtr<ID3D12Resource> mResource;
    D3D12_CPU_DESCRIPTOR_HANDLE mDescriptorHandle;
};
