#include "CommandAllocator.h"

void CommandAllocator::Reset() {
    // Reset both the command allocator and command list
    mD3D12CommandAllocator->Reset();
    mD3D12CommandList->Reset(mD3D12CommandAllocator.Get(), nullptr);

    // Bind descriptor heaps
    // TODO mD3D12CommandList->SetDescriptorHeaps();
}
