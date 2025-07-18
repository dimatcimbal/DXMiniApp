//
// Created by dtcimbal on 19/07/2025.
#include "DepthBuffer.h"

#include "Common/Debug.h"
#include "Device.h"

bool DepthBuffer::Resize(uint32_t NewWidth, uint32_t NewHeight) {
    D3D12_CLEAR_VALUE ClearValue = {};
    ClearValue.Format = mFormat;
    ClearValue.DepthStencil.Depth = 1.0f;
    ClearValue.DepthStencil.Stencil = 0;

    ComPtr<ID3D12Resource> pD3dResource;
    if (!mDevice.CreateTextureResource(NewWidth, NewHeight, 1, 1, 1, mFormat, ClearValue,
                                       D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, pD3dResource)) {
        DEBUGPRINT(L"Failed to create depth buffer resource.\n");
        return false;
    }

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = GetStencilFormat(mFormat);
    dsvDesc.Texture2D.MipSlice = 0;

    HeapHandle dsvHeapHandle(mDsvHeap);
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeapHandle.Next();
    mDevice.CreateDepthStencilView(pD3dResource, &dsvDesc, dsvHandle, mResource);
    return true;
}