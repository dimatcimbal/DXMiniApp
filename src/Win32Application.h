// src/Win32Application.h
// Created by dtcimbal on 2/06/2025.
#pragma once

#include <memory> // for std::unique_ptr

#include "Window/MainWindow.h"

class Win32Application {
  public:
    Win32Application();
    ~Win32Application();

    // No arguments, not static. It operates on its member m_pMainWindow.
    int Run() const;

  private:
    std::unique_ptr<MainWindow> mMainWindow; // Use unique_ptr for ownership
};