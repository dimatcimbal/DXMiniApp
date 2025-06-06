// src/Graphics/CommandQueue.h
// Created by dtcimbal on 9/06/2025.
#pragma once

#include <d3d12.h>
#include <mutex>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class CommandQueue {
  public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                 ComPtr<ID3D12CommandQueue>&& pCommandQueue,
                 ComPtr<ID3D12Fence>&& m_pFence)
        : m_Type(Type), m_CommandQueue(std::move(pCommandQueue)), m_pFence(std::move(m_pFence)) {};
    ~CommandQueue() = default;

    uint64_t ExecuteCommandList(ID3D12CommandList& List);
    uint64_t IncrementFence();

    ComPtr<ID3D12CommandQueue> GetCommandQueue() {
        return m_CommandQueue;
    }

    ID3D12CommandQueue* GetCommandQueueRawPtr() {
        return m_CommandQueue.Get();
    }

  private:
    /** The Queue type */
    const D3D12_COMMAND_LIST_TYPE m_Type;

    /** DX Command Queue */
    ComPtr<ID3D12CommandQueue> m_CommandQueue;

    /** Sync primitive (UINT64 version number) essentially saying "we've now processed up to version
     * 6 of the GPU's work" */
    ComPtr<ID3D12Fence> m_pFence;
    std::mutex m_FenceMutex;

    uint64_t m_NextFenceValue;
};
