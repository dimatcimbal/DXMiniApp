#include "CommandContext.h"

bool CommandContext::Create(HWND hWnd, std::unique_ptr<CommandContext>& OutContext) {

    std::unique_ptr<DebugLayer> DebugLayer;
    if (!DebugLayer::Create(DebugLayer)) {
        DEBUG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    // Find the best available D3D12 device
    std::unique_ptr<Device> Device;
    if (!Device::Create(D3D_FEATURE_LEVEL_12_0, true, true, Device)) {
        DEBUG_ERROR(L"Failed to create the Device.\n");
        return false;
    }

    // Create the Command Allocator with the Device
    std::unique_ptr<CommandAllocator> CommandAllocator;
    if (!Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        D3D12_COMMAND_LIST_FLAG_NONE, CommandAllocator)) {
        DEBUG_ERROR(L"Failed to create the Command Allocator.\n");
        return false;
    }

    // Create the Command Queue with the Device
    std::unique_ptr<CommandQueue> CommandQueue;
    if (!Device->CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_RESIDENCY_PRIORITY_HIGH,
                                    CommandQueue)) {
        DEBUG_ERROR(L"Failed to create the Command Queue.\n");
        return false;
    }

    OutContext =
        std::make_unique<CommandContext>(hWnd, std::move(CommandAllocator), std::move(CommandQueue),
                                         std::move(DebugLayer), std::move(Device));
}

bool CommandContext::OnResize(uint32_t NewWidth, uint32_t NewHeight) {
    // TODO Handle resizing logic here
    return true;
}

bool CommandContext::Draw(Camera& Camera) {
    // TODO Handle rendering logic here
    return true;
}
