#pragma once

#include <memory>
#include <wrl/client.h>

// Include DX headers
#include <Common/Debug.h>
#include "Includes/DXInclude.h"

class DebugLayer {
  public:
    static bool Create(std::unique_ptr<DebugLayer>& output);

#ifdef _DEBUG
    DebugLayer(Microsoft::WRL::ComPtr<ID3D12Debug6>&& d3dDebug,
               Microsoft::WRL::ComPtr<IDXGIDebug1>&& dxgiDebug)
        : mD3DDebug(std::move(d3dDebug)), mDXGIDebug(std::move(dxgiDebug)) {};
#endif

    // Deleted copy constructor and assignment operator to prevent copying
    DebugLayer(DebugLayer& other) = delete;
    DebugLayer& operator=(const DebugLayer& other) = delete;

    ~DebugLayer() {
        DEBUG_INFO(L"Freeing DebugLayer.\n");
#ifdef _DEBUG
        ReportLiveObjects();
#endif
    };

    void ReportLiveObjects();

#ifdef _DEBUG
  private:
    Microsoft::WRL::ComPtr<ID3D12Debug6> mD3DDebug; // DirectX debug layer interface
    Microsoft::WRL::ComPtr<IDXGIDebug1> mDXGIDebug; // DXGI debug interface
#endif
};
