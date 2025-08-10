#pragma once

#include <algorithm>
#include <wrl/client.h>
#include "Includes/DXInclude.h"

class CommandAllocator {
  public:
    CommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                     Microsoft::WRL::ComPtr<ID3D12CommandAllocator>&& pD3D12CommandAllocator,
                     Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10>&& pD3D12CommandList)
        : mType(Type), mD3D12CommandAllocator(std::move(pD3D12CommandAllocator)),
          mD3D12CommandList(std::move(pD3D12CommandList)) {
    }

    CommandAllocator(CommandAllocator& other) = delete;                  // Disable copy constructor
    CommandAllocator& operator=(const CommandAllocator& other) = delete; // Disable copy assignment

    void Reset();

  private:
    D3D12_COMMAND_LIST_TYPE mType;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mD3D12CommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList10> mD3D12CommandList;
};
