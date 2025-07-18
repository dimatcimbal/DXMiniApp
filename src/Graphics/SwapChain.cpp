//
// Created by dtcimbal on 16/07/2025.
#include "SwapChain.h"
#include "Common/Debug.h"
#include "Device/DescriptorHeap.h"
#include "Device/Device.h"

bool SwapChain::Resize(uint32_t NewWidth, uint32_t NewHeight) {

    // Resize buffers
    if FAILED (mDxgiSwapChain1->ResizeBuffers(mSwapChainBufferCount, NewWidth, NewHeight, mFormat,
                                              DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)) {
        DEBUGPRINT(L"Failed to resize swap chain buffers");
        return false;
    }

    HeapHandle rtvHeapHandle(mRtvHeap);

    // Recreate the render targets with the new dimentions
    for (uint32_t i = 0; i < mSwapChainBufferCount; ++i) {
        ComPtr<ID3D12Resource> pD3dResource;
        if (FAILED(mDxgiSwapChain1->GetBuffer(i, IID_PPV_ARGS(pD3dResource.GetAddressOf())))) {
            DEBUGPRINT(L"Failed to get swap chain buffer %d", i);
            // TODO deallocate resources
            return false;
        }
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeapHandle.Next();
        mDevice.CreateRenderTargetView(pD3dResource, nullptr, rtvHandle, mSwapChainBuffers[i]);
    }

    // Reset the current buffer index
    mCurrBuffer = 0;

    return true;
}
