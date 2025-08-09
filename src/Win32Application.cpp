// src/Win32Application.cpp
// Created by dtcimbal on 2/06/2025.
#include "Win32Application.h"
#include <Common/Debug.h>
#include <memory> // for std::unique_ptr and std::make_unique

#include "Includes/WindowsInclude.h" // Required for MSG, GetMessage, TranslateMessage, DispatchMessage

Win32Application::Win32Application() {
    // All specific window creation and child control initialization is handled by the MainWindow
    // class.
    mMainWindow = std::make_unique<MainWindow>();
    if (mMainWindow->GetHWND() == nullptr) {
        DEBUG_ERROR(L"Failed to create main window.");
    }
}

Win32Application::~Win32Application() {
    if (mMainWindow) {
        mMainWindow->Destroy();
    }
}

int Win32Application::Run() const {
    if (!mMainWindow || mMainWindow->GetHWND() == nullptr) {
        MessageBox(nullptr, L"MainWindow not initialized. Exiting.", L"Fatal Error",
                   MB_OK | MB_ICONERROR);
        return 1; // Application cannot run without a main window.
    }

    return mMainWindow->Run();
}