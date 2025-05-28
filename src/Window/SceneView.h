#pragma once

#include <windows.h>

namespace Window {
// Function prototype for the SceneView window procedure
LRESULT CALLBACK SceneViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// You might also declare other DirectX-related functions or classes here
// For example:
// void InitializeDirectX(HWND hwnd);
// void RenderDirectXScene();
// void CleanupDirectX();
} // namespace Window