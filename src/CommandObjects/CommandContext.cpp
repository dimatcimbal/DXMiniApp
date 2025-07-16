//
// Created by dtcimbal on 15/07/2025.

#include "CommandCOntext.h"
#include "Common/Debug.h"

bool CommandContext::Reset() {
    if FAILED (mCommandAlloc->Reset()) {
        DEBUGPRINT(L"Failed to reset command allocator");
        return false;
    }

    if FAILED (mCommandList->Reset(mCommandAlloc.Get(), nullptr)) {
        DEBUGPRINT(L"Failed to reset command list");
        return false;
    }

    return true;
}

bool CommandContext::SetViewportAndScissor(const D3D12_VIEWPORT& vp, const D3D12_RECT& rect) {
    if (rect.left >= rect.right || rect.top >= rect.bottom) {
        DEBUGPRINT(L"Invalid scissor rect");
        return false;
    }

    mCommandList->RSSetViewports(1, &vp);
    mCommandList->RSSetScissorRects(1, &rect);
    return true;
}
