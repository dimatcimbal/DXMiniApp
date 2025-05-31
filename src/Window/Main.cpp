// src/Window/Main.cpp
#include <windows.h> // Core Windows API functions and types
#include <gdiplus.h> // GDI+ for basic drawing (used in WinMain for startup/shutdown)
#include <CommCtrl.h> // For InitCommonControlsEx and standard control class names (WC_LISTBOX, WC_TREEVIEW)

// C++ Standard Library Includes
#include <filesystem> // For std::filesystem::path, current_path, directory_iterator

#include "FileView.h"
#include "SceneView.h"

// Link with these libraries
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Comctl32.lib") // For common controls
#pragma comment(lib, "Shlwapi.lib")  // For PathCombine, PathFindFileName

// Import GDI+ namespace
using namespace Gdiplus;

// --- Global Handles for Child Windows ---
HWND g_hwndFileView = NULL;
HWND g_hwndSceneView = NULL;
HWND g_hwndSceneTree = NULL;
HWND g_hwndSplitter1 = NULL; // Between FileList and SceneView
HWND g_hwndSplitter2 = NULL; // Between SceneView and SceneTree

// --- Constants ---
constexpr int SPLITTER_WIDTH = 6; // Width of the draggable splitter bar

// --- Menu Command IDs ---
#define IDM_EXIT 1000 // Unique ID for the Exit menu item

// --- Global Variables for Resizing ---
// Store the current desired widths of the three content panes
// These will be percentages or proportions of the total client width
float g_paneProportions[3] = {0.33f, 0.34f,
                              0.33f}; // Proportions for FileList, SceneView, SceneTree
int g_draggingSplitter = 0;           // 0: no drag, 1: dragging splitter1, 2: dragging splitter2
int g_lastMouseX = 0;                 // Last mouse X position during a drag

// --- GDI+ initialization and shutdown (remains in WinMain) ---
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

// --- Function Prototypes ---
LRESULT CALLBACK WindowProc(const HWND hwnd,
                            const UINT uMsg,
                            const WPARAM wParam,
                            const LPARAM lParam);
LRESULT CALLBACK SplitterProc(const HWND hwnd,
                              const UINT uMsg,
                              const WPARAM wParam,
                              const LPARAM lParam);

// Helper function to layout child windows
void LayoutChildWindows(const HWND hwndParent);

// Helper for registering window classes
ATOM RegisterChildWindowClass(const WNDCLASSW& wc);

// --- Helper function to lay out child windows ---
void LayoutChildWindows(const HWND hwndParent) {
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    const int clientWidth = rcClient.right - rcClient.left;
    const int clientHeight = rcClient.bottom - rcClient.top;

    // Calculate actual pixel widths based on proportions
    int fileListWidth = static_cast<int>(clientWidth * g_paneProportions[0]);
    int sceneViewWidth = static_cast<int>(clientWidth * g_paneProportions[1]);

    int xPos = 0;

    // File List
    SetWindowPos(g_hwndFileView, NULL, xPos, 0, fileListWidth, clientHeight, SWP_NOZORDER);
    xPos += fileListWidth;

    // Splitter 1
    SetWindowPos(g_hwndSplitter1, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    xPos += SPLITTER_WIDTH;

    // Scene View
    SetWindowPos(g_hwndSceneView, NULL, xPos, 0, sceneViewWidth, clientHeight, SWP_NOZORDER);
    xPos += sceneViewWidth;

    // Splitter 2
    SetWindowPos(g_hwndSplitter2, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    xPos += SPLITTER_WIDTH;

    // Scene Tree (fill remaining space to avoid gaps due to rounding)
    SetWindowPos(g_hwndSceneTree, NULL, xPos, 0, clientWidth - xPos, clientHeight, SWP_NOZORDER);
}

// Helper for registering window classes
ATOM RegisterChildWindowClass(const WNDCLASSW& wc) {
    ATOM atom = RegisterClassW(&wc);
    if (atom == 0) {
        MessageBoxW(NULL, L"Failed to register child window class!", L"Error",
                    MB_OK | MB_ICONERROR);
    }
    return atom;
}

// Window procedure for the main application window
LRESULT CALLBACK WindowProc(const HWND hwnd,
                            const UINT uMsg,
                            const WPARAM wParam,
                            const LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Initialize common controls (for TreeView and ListBox)
        INITCOMMONCONTROLSEX icex{}; // C++11 aggregate initialization
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        // --- Create the Menu Bar ---
        HMENU hMenuBar = CreateMenu();
        if (hMenuBar == NULL) {
            return -1;
        }

        // Create the "Scene" popup menu
        HMENU hSceneMenu = CreatePopupMenu();
        if (hSceneMenu == NULL) {
            return -1;
        }

        AppendMenuW(hSceneMenu, MF_STRING, IDM_EXIT, L"E&xit"); // & for accelerator key (Alt+X)
        AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSceneMenu,
                    L"&Scene"); // & for accelerator key (Alt+S)
        SetMenu(hwnd, hMenuBar);

        // --- Register Child Window Classes ---
        WNDCLASSW wcSplitter{}; // Use WNDCLASSW
        wcSplitter.lpfnWndProc = SplitterProc;
        wcSplitter.hInstance = GetModuleHandle(NULL);
        wcSplitter.lpszClassName = L"SplitterWindow";
        wcSplitter.hbrBackground = NULL; // Crucial: no default background erase
        wcSplitter.hCursor = LoadCursorW(NULL, IDC_SIZEWE);
        RegisterChildWindowClass(wcSplitter);

        // Register the SceneView window class
        WNDCLASSW wcScene{};
        wcScene.lpfnWndProc = Window::SceneViewProc; // Use the function from SceneView.cpp
        wcScene.hInstance = GetModuleHandle(NULL);
        wcScene.lpszClassName = L"SceneViewWindow";
        wcScene.hbrBackground = NULL; // Crucial: no default background erase
        RegisterChildWindowClass(wcScene);

        // --- Create Child Windows ---
        g_hwndFileView =
            CreateWindowExW(0, WC_TREEVIEW, NULL,
                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASBUTTONS |
                                TVS_HASLINES | TVS_LINESATROOT,
                            0, 0, 0, 0, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
        if (g_hwndFileView == NULL)
            return -1;
        Window::PopulateFileView(g_hwndFileView);

        g_hwndSplitter1 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);
        if (g_hwndSplitter1 == NULL)
            return -1;

        g_hwndSceneView = CreateWindowExW(0, L"SceneViewWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
        if (g_hwndSceneView == NULL)
            return -1;

        g_hwndSplitter2 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);
        if (g_hwndSplitter2 == NULL)
            return -1;

        g_hwndSceneTree =
            CreateWindowExW(0, WC_TREEVIEW, NULL,
                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASBUTTONS |
                                TVS_HASLINES | TVS_LINESATROOT,
                            0, 0, 0, 0, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
        if (g_hwndSceneTree == NULL)
            return -1;

        // Initial layout
        LayoutChildWindows(hwnd);
        return 0;
    }

    case WM_COMMAND: {
        // Handle menu commands and other control notifications
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            // When the "Exit" menu item is clicked, destroy the main window
            DestroyWindow(hwnd); // This will cause a WM_DESTROY message to be sent
            break;
        }
        return 0;
    }

    case WM_ERASEBKGND: {
        // IMPORTANT: Prevent parent window from erasing its background.
        // All visible areas will be covered by child windows or explicitly drawn by them.
        return TRUE;
    }

    case WM_PAINT: {
        // For the main window, if WM_ERASEBKGND returns TRUE, WM_PAINT will be called.
        // However, with WS_CLIPCHILDREN, the painting area passed to WM_PAINT for the parent
        // will automatically exclude the areas covered by child windows.
        // Therefore, the parent WM_PAINT generally has nothing to do if all client
        // area is covered by child windows.
        // If there were gaps, you could draw them here without double buffering,
        // because the clipping already handles it.
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        // No drawing here for the parent, as child windows cover everything.
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE: {
        // Re-layout child windows when the parent window is resized
        LayoutChildWindows(hwnd);
        return 0;
    }

    case WM_NOTIFY: {
        // Handle notifications from common controls like TreeView
        LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
        if (lpnmh->hwndFrom == g_hwndFileView && lpnmh->code == TVN_SELCHANGED) {
            // Delegate the TreeView selection handling to the FileView component
            Window::HandleFileSelection(g_hwndFileView, reinterpret_cast<LPNMTREEVIEW>(lParam),
                                        g_hwndSceneView, g_hwndSceneTree);
        }
        return 0;
    }

    case WM_DESTROY: {
        // Post a quit message to terminate the message loop
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// Window procedure for the Splitter child windows
LRESULT CALLBACK SplitterProc(const HWND hwnd,
                              const UINT uMsg,
                              const WPARAM wParam,
                              const LPARAM lParam) {
    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        // Capture mouse input to track drag operations
        SetCapture(hwnd);
        // Store initial mouse X position
        g_lastMouseX = LOWORD(lParam);
        if (hwnd == g_hwndSplitter1) {
            // Indicate dragging splitter 1
            g_draggingSplitter = 1;
        } else if (hwnd == g_hwndSplitter2) {
            // Indicate dragging splitter 2
            g_draggingSplitter = 2;
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        // If a splitter is being dragged and the left mouse button is down
        if (g_draggingSplitter != 0 && (wParam & MK_LBUTTON)) {
            const int currentMouseX = LOWORD(lParam);
            const int deltaX = currentMouseX - g_lastMouseX; // Calculate mouse movement delta

            // Get the client area of the parent window (main application window)
            RECT rcClient;
            GetClientRect(GetParent(hwnd), &rcClient);
            const int clientWidth = rcClient.right - rcClient.left;

            if (clientWidth == 0)
                return 0; // Avoid division by zero if window is minimized or too small

            if (g_draggingSplitter == 1) { // Splitter between FileList and SceneView
                // Adjust proportions based on mouse movement
                float newFileListProportion =
                    g_paneProportions[0] + static_cast<float>(deltaX) / clientWidth;
                float newSceneViewProportion =
                    g_paneProportions[1] - static_cast<float>(deltaX) / clientWidth;

                // Basic bounds checking (e.g., min 10% width)
                if (newFileListProportion > 0.1f && newSceneViewProportion > 0.1f) {
                    g_paneProportions[0] = newFileListProportion;
                    g_paneProportions[1] = newSceneViewProportion;
                    // The third pane's proportion is automatically determined by the other two
                    // to ensure they sum to 1.0 (or close to it due to float precision)
                    g_paneProportions[2] = 1.0f - (g_paneProportions[0] + g_paneProportions[1]);
                }
            } else if (g_draggingSplitter == 2) { // Splitter between SceneView and SceneTree
                // Adjust proportions based on mouse movement
                float newSceneViewProportion =
                    g_paneProportions[1] + static_cast<float>(deltaX) / clientWidth;
                float newSceneTreeProportion =
                    g_paneProportions[2] - static_cast<float>(deltaX) / clientWidth;

                // Basic bounds checking
                if (newSceneViewProportion > 0.1f && newSceneTreeProportion > 0.1f) {
                    g_paneProportions[1] = newSceneViewProportion;
                    g_paneProportions[2] = newSceneTreeProportion;
                    g_paneProportions[0] = 1.0f - (g_paneProportions[1] + g_paneProportions[2]);
                }
            }

            g_lastMouseX = currentMouseX;
            // The flicker fix ensures that the parent window's repaint is smooth.
            // Child windows will get their own redraw messages (WM_SIZE/WM_PAINT) when
            // moved/resized.
            LayoutChildWindows(GetParent(hwnd)); // Re-layout all child windows
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        ReleaseCapture();
        g_draggingSplitter = 0;
        return 0;
    }
    case WM_SETCURSOR: {
        SetCursor(LoadCursorW(NULL, IDC_SIZEWE));
        return TRUE; // Important: return TRUE to prevent DefWindowProc from setting default cursor
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw the splitter bar directly. No double buffering needed here as it's small.
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hSplitterBrush = CreateSolidBrush(RGB(100, 100, 100)); // Dark gray
        FillRect(hdc, &rc, hSplitterBrush);
        DeleteObject(hSplitterBrush);

        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        // Crucial: prevent background erase for custom-drawn splitter window
        return TRUE;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

int WINAPI WinMain(const HINSTANCE hInstance,
                   [[maybe_unused]] const HINSTANCE hPrevInstance,
                   [[maybe_unused]] const LPSTR lpCmdLine,
                   const int nCmdShow) {
    // GDI+ initialization
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    const WCHAR MAIN_CLASS_NAME[] = L"DXMiniAppWindow";

    WNDCLASSW wc{}; // C++11 aggregate initialization
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // Crucial for parent: no default background erase

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Failed to register main window class!", L"Error", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    HWND hwnd =
        CreateWindowExW(0, MAIN_CLASS_NAME, L"DXMiniApp", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                        CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700, NULL, NULL, hInstance, NULL);

    if (hwnd == NULL) {
        MessageBoxW(NULL, L"Failed to create main window!", L"Error", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg{}; // C++11 aggregate initialization
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    GdiplusShutdown(gdiplusToken);

    return static_cast<int>(msg.wParam);
}