// src/Graphics/DescriptorHeap
// Created by dtcimbal on 14/06/2025.

#include "DescriptorHeap.h"

#include "Device.h"
#include "Util/Debug.h"

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::Allocate(uint32_t Count) {
    mCurrentHandle = mCurrentHeap->GetCPUDescriptorHandleForHeapStart();

    // Copy the current handle
    D3D12_CPU_DESCRIPTOR_HANDLE pHandle = mCurrentHandle;

    // Update the offset
    mCurrentHandle.ptr += Count * mDescriptorSize;
    mNumFreeDescriptors -= Count;
    return pHandle;
}
