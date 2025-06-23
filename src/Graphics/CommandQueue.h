// src/Graphics/CommandQueue.h
// Created by dtcimbal on 9/06/2025.
#pragma once

#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>

#include "CommandList.h"

using Microsoft::WRL::ComPtr;

class CommandQueue {
  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                 ComPtr<ID3D12CommandQueue>&& pCommandQueue,
                 ComPtr<ID3D12Fence>&& mpFence)
        : mType(Type), mD3dCommandQueue(std::move(pCommandQueue)), mD3dFence(std::move(mpFence)) {};
    ~CommandQueue();

    uint64_t ExecuteCommandList(const std::unique_ptr<CommandList>& List);
    uint64_t IncrementFence();
    bool IsFenceComplete(uint64_t FenceValue);
    void WaitForFence(uint64_t FenceValue);
    void WaitForIdle(void) {
        WaitForFence(IncrementFence());
    }

    ID3D12CommandQueue* Get() {
        return mD3dCommandQueue.Get();
    }

  private:
    /** The Queue type */
    const D3D12_COMMAND_LIST_TYPE mType;

    /** DX Command Queue */
    ComPtr<ID3D12CommandQueue> mD3dCommandQueue;

    /** Sync primitive (UINT64 version number) essentially saying "we've now processed up to version
     * 6 of the GPU's work" */
    ComPtr<ID3D12Fence> mD3dFence;
    std::mutex mFenceMutex;
    std::mutex mEventMutex;
    uint64_t mLastCompletedFenceValue;
    uint64_t mNextFenceValue;
    HANDLE mFenceEventHandle;
};
