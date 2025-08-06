#pragma once

#include <memory>
#include <wrl/client.h>

// Include DX headers
#include "Includes/DxInclude.h"

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
        : mD3dDebug(std::move(d3dDebug)), mDxgiDebug(std::move(dxgiDebug)) {};
#endif

    // Deleted copy constructor and assignment operator to prevent copying
    DebugLayer(DebugLayer& other) = delete;
    DebugLayer& operator=(const DebugLayer& other) = delete;

    ~DebugLayer() {
#ifdef _DEBUG
        ReportLiveObjects();
        mDxgiDebug->Release();
        mD3dDebug->Release();
#endif
    };

    void ReportLiveObjects();
    
#ifdef _DEBUG
  private:
    Microsoft::WRL::ComPtr<ID3D12Debug6> mD3dDebug; // DirectX debug layer interface
    Microsoft::WRL::ComPtr<IDXGIDebug1> mDxgiDebug; // DXGI debug interface
#endif
};
