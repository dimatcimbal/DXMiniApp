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
                ComPtr<ID3D12CommandAllocator>&& pCommandListAlloc,
                ComPtr<ID3D12GraphicsCommandList>&& pCommandList)
        : mType(Type), mCommandListAlloc(std::move(pCommandListAlloc)),
          mCommandList(pCommandList) {};
    ~CommandList() = default;

  private:
    D3D12_COMMAND_LIST_TYPE mType;

    ComPtr<ID3D12CommandAllocator> mCommandListAlloc;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;
};
