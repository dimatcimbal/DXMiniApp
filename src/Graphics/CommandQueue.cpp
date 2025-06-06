// src/Graphics/CommandQueue.cpp
// Created by dtcimbal on 9/06/2025.
#include "CommandQueue.h"
#include <assert.h>

uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList& List) {
    std::lock_guard LockGuard(m_FenceMutex);

    assert(SUCCEEDED(static_cast<ID3D12GraphicsCommandList&>(List).Close()));

    // Get a pointer to the reference for the D3D12 API call
    ID3D12CommandList* pCommandList = &List;

    // Create a temporary array containing this single pointer.
    ID3D12CommandList* ppCommandLists[] = {pCommandList};

    // Pass the number of command lists (1) and the array.
    m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);
    m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);

    return m_NextFenceValue++;
}

uint64_t CommandQueue::IncrementFence() {
    std::lock_guard LockGuard(m_FenceMutex);
    m_CommandQueue->Signal(m_pFence.Get(), m_NextFenceValue);
    return m_NextFenceValue++;
}