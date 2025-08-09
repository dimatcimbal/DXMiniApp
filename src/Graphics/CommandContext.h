#pragma once

#include <memory>
#include "CommandQueue.h"
#include "DebugLayer.h"
#include "Device.h"
#include "Includes/WindowsInclude.h"

class Camera;

/**
 *
 *   CPU                                                                             GPU
 *    |                                                                               |
 *    | CommandQueue                                                                  |
 *    | --- CommandList(Setup, Setup, Signal(101)), CommandList(..)             --->  |
 *    |                                 |                                             |
 *    |        101                      |                                             |
 *    | <-------------------------------|                                             |
 *    |                                                                               |
 *    |                                                                               |
 *
 */
class CommandContext {
  public:
    static bool Create(HWND hWnd, std::unique_ptr<CommandContext>& OutContext);

    CommandContext(HWND hWnd,
                   std::unique_ptr<CommandAllocator> CommandAllocator,
                   std::unique_ptr<CommandQueue> CommandQueue,
                   std::unique_ptr<DebugLayer>&& DebugLayer,
                   std::unique_ptr<Device>&& Device)
        : mHWnd(hWnd), mCommandAllocator(std::move(CommandAllocator)),
          mCommandQueue(std::move(CommandQueue)), mDebugLayer(std::move(DebugLayer)),
          mDevice(std::move(Device)) {};
    ~CommandContext() = default;

    // Deleted copy constructor and assignment operator to prevent copying
    CommandContext(Device& copy) = delete;
    CommandContext& operator=(const CommandContext& copy) = delete;

    bool OnResize(uint32_t NewWidth, uint32_t NewHeight);
    bool Draw(Camera& Camera);

  private:
    std::unique_ptr<CommandAllocator> mCommandAllocator;
    std::unique_ptr<CommandQueue> mCommandQueue;
    std::unique_ptr<DebugLayer> mDebugLayer;
    std::unique_ptr<Device> mDevice;
    HWND mHWnd;
};
