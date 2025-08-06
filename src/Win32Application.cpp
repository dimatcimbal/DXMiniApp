// src/Win32Application.cpp
// Created by dtcimbal on 2/06/2025.
#include "Win32Application.h"
#include <memory>                // for std::unique_ptr and std::make_unique
#include "Includes/WindowsInclude.h" // Required for MSG, GetMessage, TranslateMessage, DispatchMessage

Win32Application::Win32Application() {
    // All specific window creation and child control initialization is handled by the MainWindow
    // class.
    mMainWindow = std::make_unique<MainWindow>();
    if (mMainWindow->GetHWND() == nullptr) {
        // TODO Handle error: main window failed to create.
    }
}

Win32Application::~Win32Application() {
    // unique_ptr automatically cleans up m_pMainWindow when Win32Application is destroyed.
}

int Win32Application::Run() const {
    if (!mMainWindow || mMainWindow->GetHWND() == nullptr) {
        MessageBox(nullptr, L"MainWindow not initialized. Exiting.", L"Fatal Error",
                   MB_OK | MB_ICONERROR);
        return 1; // Application cannot run without a main window.
    }

    return mMainWindow->Run();
}