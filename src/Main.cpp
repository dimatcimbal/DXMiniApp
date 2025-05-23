#include <tchar.h>
#include <windows.h>

// Window procedure function - handles messages sent to the window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                            LPARAM lParam) {
  switch (uMsg) {
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    // Fill the window with a light gray color
    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

    // Draw some text
    TextOut(hdc, 50, 50, _T("Hello from DXMiniApp!"), 19);

    EndPaint(hwnd, &ps);
    return 0;
  }

  default:
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
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
  HWND hwnd =
      CreateWindowEx(0,                         // Optional window styles
                     CLASS_NAME,                // Window class
                     _T("DXMiniApp"),           // Window text (title bar)
                     WS_OVERLAPPEDWINDOW,       // Window style

                     // Size and position
                     CW_USEDEFAULT, CW_USEDEFAULT, // x, y position
                     800, 600,                     // width, height

                     NULL,      // Parent window
                     NULL,      // Menu
                     hInstance, // Instance handle
                     NULL       // Additional application data
      );

  if (hwnd == NULL) {
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

  return (int)msg.wParam;
}