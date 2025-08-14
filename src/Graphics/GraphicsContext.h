#pragma once

#include <memory>
#include "CommandQueue.h"
#include "DebugLayer.h"
#include "Device.h"
#include "Includes/WindowsInclude.h"

class Camera;

/**
 * @class GraphicsContext
 * @brief Manages the core components for rendering graphics using DirectX API.
 *
 * This class encapsulates the primary objects needed for graphics rendering, including the device
 * (the abstraction of the GPU), the command queue for submitting work to the GPU, and a debug layer
 * for API validation. It orchestrates the communication between the CPU and GPU.
 *
 * ### CPU-GPU Communication Flow
 * The diagram below illustrates the asynchronous nature of the CPU-GPU relationship.
 * The CPU submits work (e.g., drawing commands) to the GPU via a `CommandQueue`. This work is
 * batched into `CommandList` objects. After submitting a command list, the CPU can signal a fence
 * with a specific value. The CPU can then continue with other tasks and later wait on the fence
 * value, which is a notification from the GPU that it has completed all the commands up to that
 * point. This allows the CPU and GPU to work in parallel, improving performance.
 *
 *   CPU                                                                 GPU
 *    |                                                                   |
 *    | CommandQueue                                                      |
 *    | --- CommandList(Setup, Setup, Signal(101)), CommandList(..) --->  |
 *    |                                 |                                 |
 *    |        101                      |                                 |
 *    | <-------------------------------|                                 |
 *    |                                                                   |
 *    |                                                                   |
 *
 */
class GraphicsContext {
  public:
    static bool Create(HWND hWnd, std::unique_ptr<GraphicsContext>& OutContext);

    GraphicsContext(HWND hWnd,
                    std::unique_ptr<CommandAllocator> CommandAllocator,
                    std::unique_ptr<CommandQueue> CommandQueue,
                    std::unique_ptr<DebugLayer>&& DebugLayer,
                    std::unique_ptr<Device>&& Device)
        : mHWnd(hWnd), mCommandAllocator(std::move(CommandAllocator)),
          mCommandQueue(std::move(CommandQueue)), mDebugLayer(std::move(DebugLayer)),
          mDevice(std::move(Device)) {};
    ~GraphicsContext() {
        // Free up mCommandAllocator.
        mCommandAllocator.reset();

        // Free up mCommandQueue.
        mCommandQueue.reset();

        // Free up mDevice.
        mDevice.reset();

        // IMPORTANT. Freeing up DebugLayer at the very last to report on LIVE objects
        // before the context is destroyed.
        mDebugLayer.reset();
    };

    // Deleted copy constructor and assignment operator to prevent copying
    GraphicsContext(Device& copy) = delete;
    GraphicsContext& operator=(const GraphicsContext& copy) = delete;

    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);
    bool Draw(Camera& Camera);

  private:
    std::unique_ptr<DebugLayer> mDebugLayer;
    std::unique_ptr<CommandAllocator> mCommandAllocator;
    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<Device> mDevice;
    HWND mHWnd;
};
