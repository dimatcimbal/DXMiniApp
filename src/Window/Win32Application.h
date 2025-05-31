// src/Window/Win32Application.h
#pragma once

#include <windows.h> // For HWND, UINT, WPARAM, LPARAM
#include <string>    // For std::wstring

namespace Window {

class Win32Application {
  public:
    Win32Application();  // Constructor
    ~Win32Application(); // Destructor

    // Static method to run the application (called by WinMain)
    static int Run(LPCWSTR windowTitle, int width, int height, HINSTANCE hInstance, int nCmdShow);

    // Static Window Procedure (the trampoline)
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Non-static method to handle messages for this specific instance
    LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    // Non-static helper for registering window classes (called on an instance)
    ATOM RegisterWindowClass(const WNDCLASSW& wc);

    // Non-static helper function to layout child windows (called on an instance)
    void LayoutChildWindows();

    // --- Window State (non-static members of the instance) ---
    HWND m_hwndMain; // The main application window handle
    HWND m_hwndFileView;
    HWND m_hwndSceneView;
    HWND m_hwndSceneTree;
    HWND m_hwndSplitter1;
    HWND m_hwndSplitter2;

    // --- Global Variables for Resizing (non-static members of the instance) ---
    static constexpr int SPLITTER_WIDTH = 6; // This can remain static if constant for all instances
    float m_paneProportions[3];
    int m_draggingSplitter;
    int m_lastMouseX;

  private:
    // Static pointer to the single instance of Win32Application.
    // Used by StaticWindowProc to get 'this' pointer.
    static Win32Application *s_pInstance;
};

// --- Forward declaration for the global SplitterProc ---
// This function must be global as it's directly assigned to a WNDCLASS.lpfnWndProc.
LRESULT CALLBACK SplitterProc(const HWND hwnd,
                              const UINT uMsg,
                              const WPARAM wParam,
                              const LPARAM lParam);

} // namespace Window