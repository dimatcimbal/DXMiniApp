// src/main.cpp
// Created by dtcimbal on 23/05/2025.
#include <Windows.h> // Required for WinMain
#include "Win32Application.h"

// The entry point for a Windows GUI application.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    Win32Application app;

    return app.Run();
}