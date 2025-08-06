//
// Created by dtcimbal on 27/07/2025.
#pragma once

#include <memory>
#include "DebugLayer.h"              // for DebugLayer
#include "Includes/WindowsInclude.h" // for HWND

class Camera;

class Device {

  public:
    static bool Create(HWND hWnd, std::unique_ptr<Device>& OutDevice);
    Device(HWND hWnd, std::unique_ptr<DebugLayer>&& DebugLayer)
        : mHwnd(hWnd), mDebugLayer(std::move(DebugLayer)) {};

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& copy) = delete;
    Device& operator=(const Device& copy) = delete;

    ~Device() = default;

    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);
    bool Draw(Camera& Camera);

  private:
    std::unique_ptr<DebugLayer> mDebugLayer;
    HWND mHwnd;
};
