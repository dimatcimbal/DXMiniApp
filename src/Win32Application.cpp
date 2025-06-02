// src/Win32Application.cpp
#include "Win32Application.h"
#include <Windows.h> // Required for MSG, GetMessage, TranslateMessage, DispatchMessage
#include <memory>    // for std::unique_ptr and std::make_unique

Win32Application::Win32Application() {
    // All specific window creation and child control initialization is handled by the MainWindow
    // class.
    m_pMainWindow =
        std::make_unique<MainWindow>(); // Corrected: m_pMainWindow is now initialized here
    if (m_pMainWindow->GetHWND() == nullptr) {
        // TODO Handle error: main window failed to create.
    }
}

Win32Application::~Win32Application() {
    // unique_ptr automatically cleans up m_pMainWindow when Win32Application is destroyed.
}

int Win32Application::Run() const {
    if (!m_pMainWindow || m_pMainWindow->GetHWND() == nullptr) {
        return 1; // Application cannot run without a main window.
    }

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}