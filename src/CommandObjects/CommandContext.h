//
// Created by dtcimbal on 15/07/2025.
//
#pragma once
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CommandContext {
  public:
    CommandContext(ComPtr<ID3D12CommandQueue>&& CommandQueue,
                   ComPtr<ID3D12CommandAllocator>&& DirectCmdListAlloc,
                   ComPtr<ID3D12GraphicsCommandList>&& CommandList)
        : mCommandQueue(CommandQueue), mCommandAlloc(DirectCmdListAlloc),
          mCommandList(CommandList) {};
    ~CommandContext() = default;

    ID3D12CommandQueue* GetCommandQueue() const {
        return mCommandQueue.Get();
    }

    bool SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect);
    bool Reset();

  private:
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    ComPtr<ID3D12CommandAllocator> mCommandAlloc;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;
};
