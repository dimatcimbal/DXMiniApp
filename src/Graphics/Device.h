// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
// #pragma comment(lib, "d3d12.lib")

using Microsoft::WRL::ComPtr;

namespace Graphics {

class Device {

  public:
    Device() = default;
    ~Device() = default;

    static ComPtr<ID3D12Device> FindBestSuitableDevice(ComPtr<IDXGIFactory6>& dxgiFactory);

    bool OnCreate();

  private:
    ComPtr<ID3D12Device> mDevice;
};

} // namespace Graphics
