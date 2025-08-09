#pragma once

#include <memory>
#include <wrl/client.h>

// Include DX headers
#include "Includes/DXInclude.h"

#include <Common/Debug.h>

// Include DX debug headers
#ifdef _DEBUG
#include <d3d12sdklayers.h>
#include <dxgidebug.h>
#endif

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
#ifdef _DEBUG
        ReportLiveObjects();

        // Release the debug interfaces
        mDXGIDebug->Release();
        mD3DDebug->Release();
#endif
    };

    void ReportLiveObjects();

#ifdef _DEBUG
  private:
    Microsoft::WRL::ComPtr<ID3D12Debug6> mD3DDebug; // DirectX debug layer interface
    Microsoft::WRL::ComPtr<IDXGIDebug1> mDXGIDebug; // DXGI debug interface
#endif
};
