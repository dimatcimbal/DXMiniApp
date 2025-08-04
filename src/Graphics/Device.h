//
// Created by dtcimbal on 27/07/2025.
#pragma once
#include "Includes/WinInclude.h" // for HWND
#include <memory>

#include "Renderer.h"

class Device {

  public:
    Device(HWND hWnd) : mHwnd(hWnd) {};
    ~Device() = default;

    bool CreateRenderer(std::unique_ptr<Renderer>& OutRenderer);

  private:
    HWND mHwnd;
};
