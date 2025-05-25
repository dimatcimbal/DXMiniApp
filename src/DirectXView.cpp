#include "DirectXView.h"
#include <string>
#include <windows.h>

// You can include DirectX specific headers here, e.g.,
// #include <d3d11.h>
// #include <DirectXMath.h>
// #pragma comment(lib, "d3d11.lib")

// Window procedure for the DirectX View child window
LRESULT CALLBACK DirectXViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        OutputDebugStringW(L"DirectXView: WM_CREATE received.\n");
        // TODO: Initialize DirectX here (e.g., D3D11 device, swap chain)
        return 0;
    }
    case WM_SIZE: {
        // TODO: Handle resizing of the DirectX view (e.g., resize swap chain buffers)
        // int width = LOWORD(lParam);
        // int height = HIWORD(lParam);
        OutputDebugStringW(L"DirectXView: WM_SIZE received.\n");
        // Invalidate to force a repaint after resize (if needed, DirectX usually renders continuously)
        InvalidateRect(hwnd, NULL, FALSE); // FALSE is key to prevent flicker here too if you're not using GDI+
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        // TODO: Perform DirectX rendering here
        // For now, draw a simple rectangle as a placeholder
        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH hBrush = CreateSolidBrush(RGB(50, 50, 150)); // A dark blue for the DirectX view
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        // Display text indicating it's the DirectX View
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        LPCWSTR text = L"DirectX View Placeholder";
        DrawTextW(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        // Crucial: Prevent background erase. DirectX (or your placeholder GDI) will draw the entire surface.
        return TRUE;

    case WM_APP_FILE_SELECTED: {
        // This message is sent from the main window when a file is selected.
        // The LPARAM contains a pointer to the wide string of the selected file path.
        const wchar_t* filePath = reinterpret_cast<const wchar_t*>(lParam);
        if (filePath) {
            std::wstring sFilePath(filePath);
            OutputDebugStringW((L"DirectXView: Received WM_APP_FILE_SELECTED: " + sFilePath + L"\n").c_str());
            // TODO: Load and render the selected file (e.g., 3D model)
            // You might need to make a copy of the string if you plan to use it asynchronously,
            // as the original string's lifetime is tied to the WM_COMMAND handler in main.cpp.
        }
        return 0;
    }
    case WM_DESTROY: {
        OutputDebugStringW(L"DirectXView: WM_DESTROY received.\n");
        // TODO: Clean up DirectX resources here
        OutputDebugStringW(L"DirectXView: WM_DESTROY received.\n");
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}
