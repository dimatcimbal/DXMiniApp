// src/main.cpp
// Created by dtcimbal on 23/05/2025.
#include "Includes/WindowsInclude.h"
#include "Window/MainWindow.h"

// The entry point for a Windows GUI application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    std::unique_ptr<MainWindow> pMainWindow;
    if (!MainWindow::Create(pMainWindow)) {
        MessageBox(nullptr, L"Failed to create Win32Application.", L"Error", MB_OK | MB_ICONERROR);
        return 1; // Exit with error code
    }

    return pMainWindow->Run();
}