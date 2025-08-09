#include "CommandQueue.h"

#include <Common/Debug.h>
#include <mutex>

bool CommandQueue::ExecuteCommandList(ID3D12GraphicsCommandList10* CommandList) {
    // std::lock_guard<std::mutex> LockGuard(mFenceValueMutex);

    if FAILED (CommandList->Close()) {
        DEBUG_ERROR("Failed to close command list.\n");
        return false; // Failed to close the command list
    }

    ID3D12CommandList* Lists[1] = {CommandList};
    mD3D12CommandQueue->ExecuteCommandLists(1, Lists);
    NextFence();
    return true;
}

uint64_t CommandQueue::NextFence() {
    std::lock_guard<std::mutex> LockGuard(mFenceValueMutex);

    // Increment the fence value and
    // uint64_t nextFenceValue = ++mFenceValue;

    // Signal the command queue with the new fence value
    mD3D12CommandQueue->Signal(mD3D12Fence.Get(), mNextFenceValue);

    // Increment after returning
    return mNextFenceValue++;
}

void CommandQueue::WaitForFence(uint64_t FenceValueToWait) {
    std::lock_guard<std::mutex> LockGuard(mFenceEventMutex);

    // Wait for the signal to complete
    if (FenceValueToWait > mD3D12Fence->GetCompletedValue()) {
        if (SUCCEEDED(mD3D12Fence->SetEventOnCompletion(FenceValueToWait, mFenceEvent))) {
            WaitForSingleObject(mFenceEvent, INFINITE);
        } else {
            DEBUG_ERROR(L"Failed to set event on fence completion for value %llu.\n",
                        FenceValueToWait);
        }
    }
}