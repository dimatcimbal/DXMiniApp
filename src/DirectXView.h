#pragma once

#include <windows.h>

// Define WM_APP_FILE_SELECTED if it's used in DirectXViewProc
#ifndef WM_APP_FILE_SELECTED
#define WM_APP_FILE_SELECTED (WM_APP + 1)
#endif

// Function prototype for the DirectXView window procedure
LRESULT CALLBACK DirectXViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// You might also declare other DirectX-related functions or classes here
// For example:
// void InitializeDirectX(HWND hwnd);
// void RenderDirectXScene();
// void CleanupDirectX();