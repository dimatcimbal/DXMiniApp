//
// Created by dtcimbal on 27/07/2025.
#pragma once

#include <memory>
#include "Includes/DxInclude.h"
#include "Includes/WindowsInclude.h" // for HWND

#include "CommandAllocator.h"
#include "CommandQueue.h"
#include "DebugLayer.h"

class Camera;

class Device {

  public:
    static bool Create(D3D_FEATURE_LEVEL FeatureLevel,
                       bool IsHardwareDevice,
                       bool HasMaxVideoMemory,
                       std::unique_ptr<Device>& OutDevice);

    bool CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE Type,
                                D3D12_COMMAND_LIST_FLAGS Flags,
                                std::unique_ptr<CommandAllocator>& OutAllocator);

    bool CreateCommandQueue(D3D12_COMMAND_LIST_TYPE Type,
                            D3D12_RESIDENCY_PRIORITY Priority,
                            std::unique_ptr<CommandQueue>& OutQueue);

    Device(Microsoft::WRL::ComPtr<IDXGIFactory7>&& DxgiFactory,
           Microsoft::WRL::ComPtr<ID3D12Device14> D3dDevice)
        : mDxgiFactory(std::move(DxgiFactory)), mD3dDevice(std::move(D3dDevice)) {};
    ~Device() = default;

    // Deleted copy constructor and assignment operator to prevent copying
    Device(Device& copy) = delete;
    Device& operator=(const Device& copy) = delete;

  private:
    Microsoft::WRL::ComPtr<IDXGIFactory7> mDxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device14> mD3dDevice;
};
