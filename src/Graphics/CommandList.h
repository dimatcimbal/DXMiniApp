// src/Graphics/CommandQueue.cpp
// Created by dtcimbal on 9/06/2025.
#pragma once
#include <d3d12.h>
#include <memory>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CommandList {
  public:
    CommandList(D3D12_COMMAND_LIST_TYPE Type,
                ComPtr<ID3D12CommandAllocator>&& pD3dCommandAlloc,
                ComPtr<ID3D12GraphicsCommandList>&& pD3dCommandList)
        : mType(Type), mD3dCommandAlloc(std::move(pD3dCommandAlloc)),
          mD3dCommandList(pD3dCommandList) {};

    ID3D12CommandList* Get() {
        return mD3dCommandList.Get();
    }

    ~CommandList() = default;

    bool Reset();

  private:
    D3D12_COMMAND_LIST_TYPE mType;

    ComPtr<ID3D12CommandAllocator> mD3dCommandAlloc;
    ComPtr<ID3D12GraphicsCommandList> mD3dCommandList;
};
