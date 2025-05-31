// src/Window/Main.cpp
#include <windows.h> // Core Windows API functions and types
#include <gdiplus.h> // GDI+ for basic drawing

// Include your new Win32Application class from the Window namespace
#include "Win32Application.h"

// Link with these libraries
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Comctl32.lib") // For common controls
#pragma comment(lib, "Shlwapi.lib")  // For PathCombine, PathFindFileName (if needed by other parts)

// Import GDI+ namespace
using namespace Gdiplus;

// --- GDI+ initialization and shutdown variables ---
// These remain global as they are process-wide GDI+ tokens.
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

int WINAPI WinMain(const HINSTANCE hInstance,
                   [[maybe_unused]] const HINSTANCE hPrevInstance,
                   [[maybe_unused]] const LPSTR lpCmdLine,
                   const int nCmdShow) {
    // 1. Initialize GDI+
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // 2. Run the application using the Win32Application framework.
    // The static Win32Application::Run method will handle:
    //   - Creating an instance of Win32Application.
    //   - Registering the main window class.
    //   - Creating the main window.
    //   - Running the Windows message loop.
    // This makes WinMain very concise and clean.
    int retCode = Window::Win32Application::Run(L"DXMiniApp", 1200, 700, hInstance, nCmdShow);

    // 3. Shutdown GDI+
    GdiplusShutdown(gdiplusToken);

    return retCode;
}