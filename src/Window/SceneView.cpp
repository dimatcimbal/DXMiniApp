// src/Window/SceneView.cpp
#include "SceneView.h"

SceneView::SceneView() = default;

SceneView::~SceneView() = default;

// Creates a simple static control as a placeholder for the scene view.
// In a real application, this would create a custom window for rendering.
bool SceneView::OnCreate(HWND hParent, UINT id) {
    m_hWnd =
        CreateWindowEx(WS_EX_CLIENTEDGE, // Extended style for a sunken border
                       L"STATIC", // Using STATIC for a placeholder; replace with your custom class
                       L"Scene View Content",             // Text for the placeholder
                       WS_CHILD | WS_VISIBLE | SS_CENTER, // Static text, centered
                       0, 0, 0, 0,                        // Position and size will be set by parent
                       hParent,                           // Parent window
                       (HMENU)(INT_PTR)id,                // Child window ID
                       GetModuleHandle(nullptr), nullptr);

    if (m_hWnd == nullptr) {
        OutputDebugString(L"Failed to create SceneView placeholder.\n");
        return false;
    }

    return true;
}

// TODO: Implement rendering loop / refresh logic here if needed.