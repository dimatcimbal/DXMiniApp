#pragma once

#include <windows.h>

// Define WM_APP_FILE_SELECTED if it's used in SceneViewProc
#ifndef WM_APP_FILE_SELECTED
#define WM_APP_FILE_SELECTED (WM_APP + 1)
#endif

namespace Window {
// Function prototype for the SceneView window procedure
LRESULT CALLBACK SceneViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// You might also declare other DirectX-related functions or classes here
// For example:
// void InitializeDirectX(HWND hwnd);
// void RenderDirectXScene();
// void CleanupDirectX();
} // namespace Window