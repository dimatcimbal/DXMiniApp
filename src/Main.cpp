#include <tchar.h>
#include <windows.h>
#include <gdiplus.h>

// Import GDI+ namespace
using namespace Gdiplus;

// Window procedure function - handles messages sent to the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Create a GDI+ Graphics object from the HDC
        Graphics graphics(hdc);

        // Enable font smoothing
        graphics.SetTextRenderingHint(TextRenderingHintAntiAlias);
        // Or for ClearType specifically:
        // graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

        // Fill the window with a light gray color
        // GDI+ doesn't use HBRUSH directly for filling like GDI.
        // Instead, you'd typically draw a filled rectangle with a SolidBrush.
        // For this simple case, the old FillRect (GDI) still works
        // because the background brush is set in the WNDCLASS.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        // Create a GDI+ SolidBrush for the text color (e.g., black)
        SolidBrush textBrush(Color(255, 0, 0, 0)); // ARGB: Opaque Black

        // Create a GDI+ Font object
        Font font(L"Arial", 24, FontStyleRegular, UnitPixel);

        // Draw some text using GDI+
        graphics.DrawString(L"Hello from DXMiniApp!", -1, &font, PointF(50.0f, 50.0f), &textBrush);

        EndPaint(hwnd, &ps);
        return 0;
    }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // GDI+ initialization
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Define the window class name
    const TCHAR CLASS_NAME[] = _T("DXMiniAppWindow");

    // Register the window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;                   // Window procedure function
    wc.hInstance = hInstance;                      // Application instance
    wc.lpszClassName = CLASS_NAME;                 // Class name
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);      // Default cursor
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // Background color

    RegisterClass(&wc);

    // Create the window
    HWND hwnd = CreateWindowEx(0,                   // Optional window styles
                               CLASS_NAME,          // Window class
                               _T("DXMiniApp"),     // Window text (title bar)
                               WS_OVERLAPPEDWINDOW, // Window style

                               // Size and position
                               CW_USEDEFAULT, CW_USEDEFAULT, // x, y position
                               800, 600,                     // width, height

                               NULL,      // Parent window
                               NULL,      // Menu
                               hInstance, // Instance handle
                               NULL       // Additional application data
    );

    if (hwnd == NULL) {
        // GDI+ shutdown even if window creation fails
        GdiplusShutdown(gdiplusToken);
        return 0; // Window creation failed
    }

    // Show and update the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // GDI+ shutdown
    GdiplusShutdown(gdiplusToken);

    return (int)msg.wParam;
}