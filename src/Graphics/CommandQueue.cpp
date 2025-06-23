// src/Graphics/CommandQueue.cpp
// Created by dtcimbal on 9/06/2025.
#define NOMINMAX // the right std::max is wanted
#include "CommandQueue.h"
#include <algorithm> // Required for std::max
#include <assert.h>
#include <stdint.h>

CommandQueue::~CommandQueue() {
    if (mD3dCommandQueue == nullptr) {
        return;
    }
    CloseHandle(mFenceEventHandle);
}

uint64_t CommandQueue::ExecuteCommandList(const std::unique_ptr<CommandList>& List) {
    std::lock_guard LockGuard(mFenceMutex);

    // Get a pointer to the reference for the D3D12 API call
    ID3D12CommandList* pCommandList = List->Get();

    assert(SUCCEEDED(static_cast<ID3D12GraphicsCommandList*>(pCommandList)->Close()));

    // Pass the number of command lists (1) and the array.
    mD3dCommandQueue->ExecuteCommandLists(1, &pCommandList);
    mD3dCommandQueue->Signal(mD3dFence.Get(), mNextFenceValue);

    return mNextFenceValue++;
}

uint64_t CommandQueue::IncrementFence() {
    std::lock_guard LockGuard(mFenceMutex);
    mD3dCommandQueue->Signal(mD3dFence.Get(), mNextFenceValue);
    return mNextFenceValue++;
}

bool CommandQueue::IsFenceComplete(uint64_t FenceValue) {
    // Avoid querying the fence value by testing against the last one seen.
    // The max() is to protect against an unlikely race condition that could cause the last
    // completed fence value to regress.
    if (FenceValue > mLastCompletedFenceValue) {
        mLastCompletedFenceValue =
            std::max(mLastCompletedFenceValue, mD3dFence->GetCompletedValue());
    }
    return FenceValue <= mLastCompletedFenceValue;
}

void CommandQueue::WaitForFence(uint64_t FenceValue) {
    // Check if already completed
    if (IsFenceComplete(FenceValue)) {
        return;
    }

    // the LockGuard scope; Async wait
    {
        std::lock_guard<std::mutex> LockGuard(mEventMutex);

        mD3dFence->SetEventOnCompletion(FenceValue, mFenceEventHandle);
        WaitForSingleObject(mFenceEventHandle, INFINITE);
        mLastCompletedFenceValue = FenceValue;
    }
}