//
// Created by dtcimbal on 15/07/2025.
//

#include "Renderer.h"

#include "CommandObjects/CommandContext.h"
#include "CommandObjects/LocalContext.h"
#include "Common/Debug.h"
#include "DepthBuffer.h"
#include "SwapChain.h"

bool Renderer::Draw(const std::unique_ptr<Camera>& pCamera) {

    LocalContext context{mCommandContext};

    if (!context) {
        DEBUGPRINT(L"Failed to reset local command context.\n");
        return false;
    }



    // Draw call below
    context->SetViewportAndScissor(mScreenViewport, mScissorRect);
    // TODO Use the Box app example to complete this




    return true;
}

bool Renderer::OnResize(uint32_t Width, uint32_t Height) {

    mCommandContext->WaitForIdle(); // FlushCommandQueue

    if (!mSwapChain->Resize(Width, Height)) {
        DEBUGPRINT(L"Failed to resize swap chain buffers.\n");
        return false;
    }

    if (!mDepthBuffer->Resize(Width, Height)) {
        DEBUGPRINT(L"Failed to resize depth buffer.\n");
        return false;
    }

    mCommandContext->WaitForIdle();

    // Update viewport
    mScreenViewport.TopLeftX = 0;
    mScreenViewport.TopLeftY = 0;
    mScreenViewport.Width = static_cast<float>(Width);
    mScreenViewport.Height = static_cast<float>(Height);
    mScreenViewport.MinDepth = 0.0f;
    mScreenViewport.MaxDepth = 1.0f;

    // update scissor rect
    mScissorRect.left = 0;
    mScissorRect.top = 0;
    mScissorRect.right = static_cast<LONG>(Width);
    mScissorRect.bottom = static_cast<LONG>(Height);

    return false;
}