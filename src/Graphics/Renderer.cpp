//
// Created by dtcimbal on 15/07/2025.
//

#include "Renderer.h"

#include "CommandObjects/CommandContext.h"
#include "CommandObjects/LocalContext.h"
#include "Common/Debug.h"
#include "DescriptorHeap.h"
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

    // mCommandContext.WaitForIdle();

    mSwapChain->OnResize(Width, Height);

    // mCommandContext.WaitForIdle();

    return false;
}