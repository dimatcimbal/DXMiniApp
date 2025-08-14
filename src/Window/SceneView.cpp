// src/Window/SceneView.cpp
// Created by dtcimbal on 26/05/2025.
#include "SceneView.h"
#include <Graphics/GraphicsContext.h>
#include <sstream> // For std::wostringstream
#include <string>  // For std::to_wstring

#include "Common/Debug.h" // For DEBUGPRINT
#include "Graphics/Device.h"
#include "Scene/Camera.h"

SceneView::SceneView() = default;

SceneView::~SceneView() = default;

// Registers the window class for the SceneView window.
bool SceneView::RegisterWindowClass() {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OnWindowMessage;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = SCENE_VIEW_CLASS_NAME;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    // Explicitly set hbrBackground to nullptr to prevent flicker and indicate custom drawing.
    wc.hbrBackground = nullptr;

    ATOM atom = RegisterClassEx(&wc);
    if (atom == 0) {
        DEBUG_ERROR(L"RegisterClassEx failed for %s. Error: %s", SCENE_VIEW_CLASS_NAME,
                    std::to_wstring(GetLastError()).c_str());
        return false;
    }
    mAtom = atom;
    return true;
}

// OmWindowMessage
LRESULT CALLBACK SceneView::OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
        OnResize(width, height);
        return 0; // Message handled
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        OnUpdate();
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
        DEBUG_ERROR(L"Failed to register SceneView window class!\n");
        return false;
    }

    // 2. Create the actual Win32 window
    mHWnd = CreateWindowEx(
        0,                     // Optional extended styles
        SCENE_VIEW_CLASS_NAME, // Name of the registered window class
        nullptr,               // No window title (for a child control)
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, // Basic child window styles
        0, 0, 0, 0,                  // Position and size set by parent (MainWindow)
        hParent,                     // Parent window handle
        reinterpret_cast<HMENU>(id), // Child window ID
        GetModuleHandle(nullptr),    // Instance handle
        this                         // Pointer to 'this' passed as creation parameter
    );

    if (mHWnd == nullptr) {
        DEBUG_ERROR(L"Failed to create SceneView window. Error: %s",
                    std::to_wstring(GetLastError()).c_str());
        return false;
    }

    // 3. Get initial client area dimensions
    RECT clientRect;
    if (!GetClientRect(mHWnd, &clientRect)) {
        DEBUG_ERROR(L"Failed to get client rect for SceneView window! Error: %s",
                    std::to_wstring(GetLastError()).c_str());
        DestroyWindow(mHWnd); // Clean up partially created window
        return false;
    }

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    mCamera = std::make_unique<Camera>();

    if (!GraphicsContext::Create(mHWnd, mGraphicsContext)) {
        DEBUG_ERROR(L"Failed to initialize a Device.\n");
        return false;
    }

    mGraphicsContext->OnResize(width, height);
    return true;
}

void SceneView::OnDestroy() {
    if (mAtom) {
        UnregisterClass(reinterpret_cast<LPCWSTR>(mAtom), GetModuleHandle(nullptr));
    }
}

// Handles WM_SIZE messages for the SceneView window.
void SceneView::OnResize(int Width, int Height) const {
    if (mGraphicsContext && Width > 0 && Height > 0) {
        mGraphicsContext->OnResize(Width, Height);
    }
}

void SceneView::OnUpdate() const {
    if (mGraphicsContext && mCamera) {
        mGraphicsContext->Draw(*mCamera);
    }
}
