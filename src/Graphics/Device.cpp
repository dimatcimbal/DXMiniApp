//
// Created by dtcimbal on 27/07/2025.
#include "Device.h"

#include <Common/Debug.h>
#include <Scene/Camera.h>

#include "DebugLayer.h"

bool Device::Create(HWND hWnd, std::unique_ptr<Device>& OutDevice) {
    if (!hWnd) {
        DEBUG_ERROR(L"Invalid window handle passed to Device::Create.\n");
        return false; // Invalid window handle
    }

    std::unique_ptr<DebugLayer> DebugLayer;
    if (!DebugLayer::Create(DebugLayer)) {
        DEBUG_ERROR(L"Failed to initialize the Debug Layer.\n");
        return false;
    }

    OutDevice = std::make_unique<Device>(hWnd, std::move(DebugLayer));
    return true;
}

bool Device::OnResize(uint32_t NewWidth, uint32_t NewHeight) {
    // TODO Handle resizing logic here
    return true;
}

bool Device::Draw(Camera& Camera) {
    // TODO Handle rendering logic here
    return true;
}
