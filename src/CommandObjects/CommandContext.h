//
// Created by dtcimbal on 15/07/2025.
//
#pragma once
#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>
#include "CommandQueue.h"

using Microsoft::WRL::ComPtr;

class CommandQueue;

class CommandContext {
  public:
    CommandContext(std::unique_ptr<CommandQueue>&& CommandQueue,
                   ComPtr<ID3D12CommandAllocator>&& DirectCmdListAlloc,
                   ComPtr<ID3D12GraphicsCommandList>&& CommandList)
        : mCommandQueue(std::move(CommandQueue)), mCommandAlloc(DirectCmdListAlloc),
          mCommandList(CommandList) {};
    ~CommandContext() = default;

    ID3D12CommandQueue* CommandContext::GetCommandQueue() const {
        return mCommandQueue->GetCommandQueue();
    }

    bool SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect);
    bool Reset();

    void WaitForIdle() {
        mCommandQueue->WaitForFence(mCommandQueue->IncrementFence());
    };

  private:
    std::unique_ptr<CommandQueue> mCommandQueue;
    ComPtr<ID3D12CommandAllocator> mCommandAlloc;
    ComPtr<ID3D12GraphicsCommandList> mCommandList;
};
