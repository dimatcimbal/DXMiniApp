// src/Graphics/Device.cpp
// Created by dtcimbal on 6/06/2025.
#include "Device.h"

#include <cstdint>
#include <dxgi.h>
#include <d3d12.h>
#include <memory>

namespace Graphics {

bool Device::OnCreate() {

    ComPtr<IDXGIFactory6> dxgiFactory;
    if FAILED (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory))) {
        OutputDebugString(L"Failed to create DXGI factory.\n");
        return false;
    }

    ComPtr<ID3D12Device> pDevice;
    pDevice = FindBestSuitableDevice(dxgiFactory);

    if (!pDevice) {
        OutputDebugString(L"No suitable DXGI adapter and D3D12 device found.\n");
        return false;
    }

    if (mDevice != nullptr) {
        mDevice->Release();
    }

    mDevice = std::move(pDevice);

    return true;
}

ComPtr<ID3D12Device> Device::FindBestSuitableDevice(ComPtr<IDXGIFactory6>& dxgiFactory) {
    SIZE_T MaxMemory = 0;

    ComPtr<IDXGIAdapter1> pAdapter;
    ComPtr<ID3D12Device> pDevice;
    ComPtr<ID3D12Device> pResult;

    // Loop through all available adapters
    for (uint32_t i = 0; dxgiFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC1 desc;
        pAdapter->GetDesc1(&desc);

        // Hardware adapter
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            pAdapter.Reset();
            continue;
        }

        // An adapter that can create a D3D12 device
        if (FAILED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)))) {
            pAdapter.Reset();
            pDevice.Reset();
            continue;
        }

        // An adapter with the *most* dedicated video memory
        if (desc.DedicatedVideoMemory > MaxMemory) {
            MaxMemory = desc.DedicatedVideoMemory;
            pResult = std::move(pDevice);

        } else {
            pDevice.Reset();
        }

        pAdapter.Reset();

    }

    return pResult;
}

} // namespace Graphics