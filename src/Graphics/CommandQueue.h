#pragma once

#include <memory>
#include "Includes/DxInclude.h"
#include "Includes/WindowsInclude.h"

#include <mutex>

class CommandQueue {
  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                 uint64_t InitFenceValue,
                 HANDLE FenceEvent,
                 Microsoft::WRL::ComPtr<ID3D12CommandQueue>&& pD3D12CommandQueue,
                 Microsoft::WRL::ComPtr<ID3D12Fence1>&& pD3D12Fence)
        : mType(Type), mNextFenceValue(InitFenceValue), mFenceEvent(FenceEvent),
          mD3D12CommandQueue(std::move(pD3D12CommandQueue)), mD3D12Fence(std::move(pD3D12Fence)) {
    }
    ~CommandQueue() {
        if (mFenceEvent) {
            CloseHandle(mFenceEvent);
        }

        mD3D12CommandQueue->Release();
        mD3D12Fence->Release();
    };

    // Deleted copy constructor and assignment operator to prevent copying
    CommandQueue(CommandQueue& copy) = delete;
    CommandQueue& operator=(const CommandQueue& copy) = delete;

    // Functions
    bool ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList);

    uint64_t NextFence();

    void WaitForFence(uint64_t FenceValueToWait);
    void WaitForIdle() {
        WaitForFence(NextFence());
    }

  private:
    D3D12_COMMAND_LIST_TYPE mType;
    uint64_t mNextFenceValue;
    std::mutex mFenceValueMutex;

    HANDLE mFenceEvent;
    std::mutex mFenceEventMutex;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> mD3D12CommandQueue;
    Microsoft::WRL::ComPtr<ID3D12Fence1> mD3D12Fence;
};
