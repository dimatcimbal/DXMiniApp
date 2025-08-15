#include "GraphicsContext.h"

bool GraphicsContext::Create(std::unique_ptr<GraphicsContext>& OutContext) {

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

    // Create the Command Queue with the Device
    std::unique_ptr<CommandQueue> CommandQueue;
    if (!Device->CreateCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                    D3D12_COMMAND_QUEUE_PRIORITY_HIGH,
                                    D3D12_COMMAND_QUEUE_FLAG_NONE, CommandQueue)) {
        DEBUG_ERROR(L"Failed to create the Command Queue.\n");
        return false;
    }

    // Create the Command Allocator with the Device
    std::unique_ptr<CommandAllocator> CommandAllocator;
    if (!Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        D3D12_COMMAND_LIST_FLAG_NONE, CommandAllocator)) {
        DEBUG_ERROR(L"Failed to create the Command Allocator.\n");
        return false;
    }

    OutContext =
        std::make_unique<GraphicsContext>(std::move(CommandAllocator), std::move(CommandQueue),
                                          std::move(DebugLayer), std::move(Device));
    return true;
}

bool GraphicsContext::OnResize(uint32_t NewWidth, uint32_t NewHeight) {
    // TODO Handle resizing logic here
    return true;
}

bool GraphicsContext::Draw(Camera& Camera) {
    // TODO Handle rendering logic here
    return true;
}
