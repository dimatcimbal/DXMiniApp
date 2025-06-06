// src/Window/SceneView.cpp
// Created by dtcimbal on 26/05/2025.
#include "SceneView.h"
#include <sstream> // For std::wostringstream
#include <stdexcept>
#include <string> // For std::to_wstring

SceneView::SceneView() = default;

SceneView::~SceneView() = default;

// Registers the window class for the SceneView window.
ATOM SceneView::RegisterWindowClass() {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = StaticWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = SCENE_VIEW_CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Default background

    ATOM atom = RegisterClassEx(&wc);
    if (atom == 0) {
        std::wostringstream errorMessage;
        errorMessage << L"RegisterClassEx failed for SCENE_VIEW_CLASS_NAME. Error: "
                     << std::to_wstring(GetLastError()) << L"\n";
        OutputDebugString(errorMessage.str().c_str());
    }
    return atom;
}

// Static Window Procedure (Trampoline) for SceneView:
LRESULT CALLBACK SceneView::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SceneView* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<SceneView*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<SceneView*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Instance-specific message handler for SceneView
LRESULT SceneView::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        OnSize(width, height);
        return 0; // Message handled
    }
    case WM_CREATE: {
        // Here you would typically do any SceneView-specific creation.
        // The mDevice is already created in OnCreate of the public interface.
        return 0;
    }
    case WM_PAINT: {
        // Basic paint handling to ensure the window is drawn.
        // Your Graphics::Device will handle the actual rendering.
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        // Return TRUE to prevent default background erasing and reduce flicker.
        return TRUE;
    }
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// Creates the SceneView window and initializes the graphics device.
bool SceneView::OnCreate(HWND hParent, UINT id) {
    // Register the custom window class for SceneView
    if (!RegisterWindowClass()) {
        OutputDebugString(L"ERROR: Failed to register SceneView window class!\n");
        return false;
    }

    mHWnd = CreateWindowEx(
        0,                     // Optional extended styles
        SCENE_VIEW_CLASS_NAME, // Name of the registered window class
        nullptr,               // No window title (for a child control)
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Basic child window styles
        0, 0, 0, 0,               // Position and size set by parent (MainWindow)
        hParent,                  // Parent window handle
        (HMENU)(INT_PTR)id,       // Child window ID
        GetModuleHandle(nullptr), // Instance handle
        this                      // Pointer to 'this' passed as creation parameter
    );

    if (mHWnd == nullptr) {
        OutputDebugString(L"Failed to create SceneView window.\n");
        return false;
    }

    mDevice = std::make_unique<Graphics::Device>(mHWnd);
    if FAILED (!mDevice->OnCreate(0, 0)) {
        OutputDebugString(L"Failed to initialize a Graphics::Device.\n");
        // Consider destroying the window here if device creation is critical
        DestroyWindow(mHWnd);
        return false;
    }

    return true;
}

// Handles WM_SIZE messages for the SceneView window.
void SceneView::OnSize(int displayWidth, int displayHeight) {
    if (mDevice) {
        // Pass the new dimensions to the graphics device for display resize.
        mDevice->DisplayResize(displayWidth, displayHeight);
    }
}