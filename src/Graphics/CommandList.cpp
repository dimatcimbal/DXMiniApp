// src/Graphics/CommandQueue.cpp
// Created by dtcimbal on 9/06/2025.

#include "CommandList.h"

#include "Util/Debug.h"

bool CommandList::Reset() {
    if FAILED (mD3dCommandList->Reset(mD3dCommandAlloc.Get(), nullptr)) {
        DEBUGPRINT(L"Failed to reset ID3D12GraphicsCommandList.\n");
        return false;
    }

    return true;
}
