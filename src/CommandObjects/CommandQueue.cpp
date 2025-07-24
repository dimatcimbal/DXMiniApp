//
// Created by dtcimbal on 25/07/2025.
//

#include "CommandQueue.h"

//
uint64_t CommandQueue::IncrementFence() {
    std::lock_guard<std::mutex> LockGuard(mFenceMutex);

    mCommandQueue->Signal(mFence.Get(), ++mCurrentFence);
    return mCurrentFence;
}

void CommandQueue::WaitForFence(uint64_t FenceValue) {
    std::lock_guard<std::mutex> LockGuard(mFenceMutex);

    if (mFence->GetCompletedValue() < FenceValue) {
        mFence->SetEventOnCompletion(FenceValue, mFenceEventHandle);
        WaitForSingleObject(mFenceEventHandle, INFINITE);
    }
}
