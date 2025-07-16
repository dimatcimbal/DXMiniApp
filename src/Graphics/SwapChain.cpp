//
// Created by dtcimbal on 16/07/2025.
#include "SwapChain.h"
#include "Common/Debug.h"
#include "Resource.h"

bool SwapChain::OnResize(uint32_t NewWidth, uint32_t NewHeight) {

    // Resize buffers
    if FAILED (mSwapChain1->ResizeBuffers(SwapChainBufferCount, NewWidth, NewHeight,
                                          mBackBufferFormat,
                                          DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)) {
        DEBUGPRINT(L"Failed to resize swap chain buffers");
        return false;
    }

    HeapHandle rtvHeapHandle(mRtvHeap);

    //
    for (uint32_t i = 0; i < SwapChainBufferCount; ++i) {
        ComPtr<ID3D12Resource> DisplayPlane;
        if (FAILED(mSwapChain1->GetBuffer(i, IID_PPV_ARGS(&DisplayPlane)))) {
            DEBUGPRINT(L"Failed to get swap chain buffer %d", i);
            return false;
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeapHandle.Next();
        mD3dDevice->CreateRenderTargetView(DisplayPlane.Get(), nullptr, rtvHandle);
        mSwapChainBuffer[i] = std::make_unique<Resource>(rtvHandle, std::move(DisplayPlane));
    }

    // Reset the current buffer index
    mCurrBuffer = 0;

    return true;
}
