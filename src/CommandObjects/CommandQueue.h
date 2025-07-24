//
// Created by dtcimbal on 25/07/2025.
//
#pragma once

#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CommandQueue {
  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                 ComPtr<ID3D12Fence1>&& Fence,
                 ComPtr<ID3D12CommandQueue>&& CommandQueue)
        : mType(Type), mFence(Fence),
          mFenceEventHandle(CreateEvent(nullptr, false, false, nullptr)),
          mCommandQueue(CommandQueue) {
    }

    ~CommandQueue() {
        if (mFenceEventHandle) {
            CloseHandle(mFenceEventHandle);
        }
    }

    ID3D12CommandQueue* GetCommandQueue() const {
        return mCommandQueue.Get();
    }

    uint64_t IncrementFence();
    void WaitForFence(uint64_t FenceValue);
    void WaitForIdle() {
        WaitForFence(IncrementFence());
    };

  private:
    D3D12_COMMAND_LIST_TYPE mType;
    uint64_t mCurrentFence{0};
    std::mutex mFenceMutex;
    ComPtr<ID3D12Fence1> mFence;
    ComPtr<ID3D12CommandQueue> mCommandQueue;
    HANDLE mFenceEventHandle;
};
