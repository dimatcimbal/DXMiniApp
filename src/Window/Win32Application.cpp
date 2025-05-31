// src/Window/Win32Application.cpp
#include "Win32Application.h"
#include <CommCtrl.h>  // For InitCommonControlsEx, WC_TREEVIEW
#include <stdexcept>   // For std::runtime_error
#include <string>      // For std::to_wstring
#include "FileView.h"  // For Window::PopulateFileView, Window::HandleFileSelection
#include "SceneView.h" // For Window::SceneViewProc

// --- Define Menu Command IDs ---
#define IDM_EXIT 1000 // Unique ID for the Exit menu item

namespace Window {

// Initialize the static instance pointer
Win32Application *Win32Application::s_pInstance = nullptr;

// --- Win32Application Constructor ---
Win32Application::Win32Application()
    : m_hwndMain(NULL), m_hwndFileView(NULL), m_hwndSceneView(NULL), m_hwndSceneTree(NULL),
      m_hwndSplitter1(NULL), m_hwndSplitter2(NULL), m_draggingSplitter(0), m_lastMouseX(0) {
    // Initialize pane proportions
    m_paneProportions[0] = 0.33f;
    m_paneProportions[1] = 0.34f;
    m_paneProportions[2] = 0.33f;

    // Set the static instance pointer (only one instance allowed for this pattern)
    // Add error handling if trying to create multiple instances, though in this design
    // it's created once in static Run.
    // If s_pInstance is not nullptr, it implies an attempt to create a second instance.
    if (s_pInstance != nullptr) {
        // This indicates a design flaw if it happens outside of expected singleton management.
        // For this specific Run() design, it should only be set once during the 'appInstance'
        // creation. It's technically okay for the constructor to set it if Run() guarantees only
        // one temporary object.
    }
    s_pInstance = this; // Set the static pointer to this new instance
}

// --- Win32Application Destructor ---
Win32Application::~Win32Application() {
    // Clear the static instance pointer when this instance is destroyed
    if (s_pInstance == this) { // Only clear if this is the active instance
        s_pInstance = nullptr;
    }
}

// --- Non-static helper for registering window classes ---
// Now called on an instance: appInstance.RegisterWindowClass(wc)
ATOM Win32Application::RegisterWindowClass(const WNDCLASSW& wc) {
    ATOM atom = RegisterClassW(&wc);
    if (atom == 0) {
        std::wstring errorMsg =
            L"Failed to register window class! Error: " + std::to_wstring(GetLastError());
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
    return atom;
}

// --- Non-static helper function to lay out child windows ---
// Now called on an instance: appInstance.LayoutChildWindows() or pApp->LayoutChildWindows()
void Win32Application::LayoutChildWindows() {
    if (!m_hwndMain)
        return; // Ensure main window exists

    RECT rcClient;
    GetClientRect(m_hwndMain, &rcClient);
    const int clientWidth = rcClient.right - rcClient.left;
    const int clientHeight = rcClient.bottom - rcClient.top;

    int fileListWidth = static_cast<int>(clientWidth * m_paneProportions[0]);
    int sceneViewWidth = static_cast<int>(clientWidth * m_paneProportions[1]);

    int xPos = 0;

    if (m_hwndFileView) {
        SetWindowPos(m_hwndFileView, NULL, xPos, 0, fileListWidth, clientHeight, SWP_NOZORDER);
    }
    xPos += fileListWidth;

    if (m_hwndSplitter1) {
        SetWindowPos(m_hwndSplitter1, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    }
    xPos += SPLITTER_WIDTH;

    if (m_hwndSceneView) {
        SetWindowPos(m_hwndSceneView, NULL, xPos, 0, sceneViewWidth, clientHeight, SWP_NOZORDER);
    }
    xPos += sceneViewWidth;

    if (m_hwndSplitter2) {
        SetWindowPos(m_hwndSplitter2, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    }
    xPos += SPLITTER_WIDTH;

    if (m_hwndSceneTree) {
        SetWindowPos(m_hwndSceneTree, NULL, xPos, 0, clientWidth - xPos, clientHeight,
                     SWP_NOZORDER);
    }
}

// --- Static Run method (called by WinMain) ---
int Win32Application::Run(
    LPCWSTR windowTitle, int width, int height, HINSTANCE hInstance, int nCmdShow) {
    // Create the single instance of Win32Application on the stack.
    // Its constructor will automatically set s_pInstance.
    Win32Application appInstance;

    const WCHAR MAIN_CLASS_NAME[] = L"DXMiniAppWindow";

    // Register the main window class
    WNDCLASSW wc{};
    wc.lpfnWndProc = StaticWindowProc; // Points to our static trampoline
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // Crucial: no default background erase

    if (!appInstance.RegisterWindowClass(wc)) { // Call non-static method on appInstance
        return 1;                               // Registration failed
    }

    // Create the main window
    // IMPORTANT: Pass '&appInstance' (the 'this' pointer of our object) as lpParam.
    // This allows StaticWindowProc to retrieve the instance later.
    appInstance.m_hwndMain = CreateWindowExW(
        0, MAIN_CLASS_NAME, windowTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT,
        CW_USEDEFAULT, width, height, NULL, NULL, hInstance, &appInstance); // Pass 'this' pointer

    if (appInstance.m_hwndMain == NULL) {
        std::wstring errorMsg =
            L"Failed to create main window! Error: " + std::to_wstring(GetLastError());
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(appInstance.m_hwndMain, nCmdShow);
    UpdateWindow(appInstance.m_hwndMain);

    MSG msg{};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // The destructor of appInstance will be called here as it goes out of scope.
    return static_cast<int>(msg.wParam);
}

// --- Static Window Procedure (The Trampoline) ---
LRESULT CALLBACK Win32Application::StaticWindowProc(HWND hwnd,
                                                    UINT uMsg,
                                                    WPARAM wParam,
                                                    LPARAM lParam) {
    // Get the Win32Application instance associated with this window.
    // On WM_NCCREATE, the pointer is in lParam (lpCreateParams).
    // For all other messages, it's stored in GWLP_USERDATA.
    Win32Application *pApp = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW *pCreate = reinterpret_cast<CREATESTRUCTW *>(lParam);
        pApp = static_cast<Win32Application *>(pCreate->lpCreateParams);
        // Store the pointer in the window's user data for future messages.
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pApp));
    } else {
        pApp = reinterpret_cast<Win32Application *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    // If we have an application instance, let it handle the message.
    if (pApp) {
        return pApp->HandleMessage(hwnd, uMsg, wParam, lParam);
    }

    // Default processing for messages not handled by the application or if no app instance.
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// --- Non-static HandleMessage (Contains your original WindowProc logic) ---
LRESULT Win32Application::HandleMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Initialize common controls (for TreeView and ListBox)
        INITCOMMONCONTROLSEX icex{};
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        // --- Create the Menu Bar ---
        HMENU hMenuBar = CreateMenu();
        if (hMenuBar == NULL)
            return -1;
        HMENU hSceneMenu = CreatePopupMenu();
        if (hSceneMenu == NULL)
            return -1;
        AppendMenuW(hSceneMenu, MF_STRING, IDM_EXIT, L"E&xit");
        AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSceneMenu, L"&Scene");
        SetMenu(hwnd, hMenuBar);

        // --- Register Child Window Classes ---
        WNDCLASSW wcSplitter{};
        wcSplitter.lpfnWndProc = SplitterProc; // This is the global SplitterProc (see below)
        wcSplitter.hInstance = GetModuleHandle(NULL);
        wcSplitter.lpszClassName = L"SplitterWindow";
        wcSplitter.hbrBackground = NULL;
        wcSplitter.hCursor = LoadCursorW(NULL, IDC_SIZEWE);
        if (!RegisterWindowClass(wcSplitter))
            return -1; // Call non-static method

        WNDCLASSW wcScene{};
        wcScene.lpfnWndProc = Window::SceneViewProc; // From SceneView.cpp
        wcScene.hInstance = GetModuleHandle(NULL);
        wcScene.lpszClassName = L"SceneViewWindow";
        wcScene.hbrBackground = NULL;
        if (!RegisterWindowClass(wcScene))
            return -1; // Call non-static method

        // --- Create Child Windows ---
        m_hwndFileView =
            CreateWindowExW(0, WC_TREEVIEW, NULL,
                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASBUTTONS |
                                TVS_HASLINES | TVS_LINESATROOT,
                            0, 0, 0, 0, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
        if (m_hwndFileView == NULL)
            return -1;
        Window::PopulateFileView(m_hwndFileView);

        // For splitter windows, we don't need to pass 'this' pointer as lpParam
        // because the global SplitterProc will retrieve the Win32Application instance
        // from the *parent* window's GWLP_USERDATA.
        m_hwndSplitter1 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);
        if (m_hwndSplitter1 == NULL)
            return -1;

        m_hwndSceneView = CreateWindowExW(0, L"SceneViewWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
        if (m_hwndSceneView == NULL)
            return -1;

        m_hwndSplitter2 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);
        if (m_hwndSplitter2 == NULL)
            return -1;

        m_hwndSceneTree =
            CreateWindowExW(0, WC_TREEVIEW, NULL,
                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASBUTTONS |
                                TVS_HASLINES | TVS_LINESATROOT,
                            0, 0, 0, 0, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
        if (m_hwndSceneTree == NULL)
            return -1;

        LayoutChildWindows(); // Call the non-static method on 'this' instance
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDM_EXIT:
            DestroyWindow(hwnd);
            break;
        }
        return 0;
    }

    case WM_ERASEBKGND: {
        return TRUE;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_SIZE: {
        LayoutChildWindows(); // Call the non-static method on 'this' instance
        return 0;
    }

    case WM_NOTIFY: {
        LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
        if (lpnmh->hwndFrom == m_hwndFileView && lpnmh->code == TVN_SELCHANGED) {
            Window::HandleFileSelection(m_hwndFileView, reinterpret_cast<LPNMTREEVIEW>(lParam),
                                        m_hwndSceneView, m_hwndSceneTree);
        }
        return 0;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

LRESULT CALLBACK SplitterProc(const HWND hwnd,
                              const UINT uMsg,
                              const WPARAM wParam,
                              const LPARAM lParam) {
    // Get the Win32Application instance from the main parent window
    // (the splitter's parent is the main app window).
    HWND hwndParent = GetParent(hwnd);
    Window::Win32Application *pApp =
        reinterpret_cast<Window::Win32Application *>(GetWindowLongPtr(hwndParent, GWLP_USERDATA));

    if (!pApp) {
        // This should not happen if the main window was created correctly.
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        SetCapture(hwnd);
        pApp->m_lastMouseX = LOWORD(lParam);        // Access non-static member
        if (hwnd == pApp->m_hwndSplitter1) {        // Access non-static member
            pApp->m_draggingSplitter = 1;           // Access non-static member
        } else if (hwnd == pApp->m_hwndSplitter2) { // Access non-static member
            pApp->m_draggingSplitter = 2;           // Access non-static member
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (pApp->m_draggingSplitter != 0 && (wParam & MK_LBUTTON)) {
            const int currentMouseX = LOWORD(lParam);
            const int deltaX = currentMouseX - pApp->m_lastMouseX;

            RECT rcClient;
            GetClientRect(GetParent(hwnd), &rcClient);
            const int clientWidth = rcClient.right - rcClient.left;

            if (clientWidth == 0)
                return 0;

            if (pApp->m_draggingSplitter == 1) {
                float newFileListProportion =
                    pApp->m_paneProportions[0] + static_cast<float>(deltaX) / clientWidth;
                float newSceneViewProportion =
                    pApp->m_paneProportions[1] - static_cast<float>(deltaX) / clientWidth;

                if (newFileListProportion > 0.1f && newSceneViewProportion > 0.1f) {
                    pApp->m_paneProportions[0] = newFileListProportion;
                    pApp->m_paneProportions[1] = newSceneViewProportion;
                    pApp->m_paneProportions[2] =
                        1.0f - (pApp->m_paneProportions[0] + pApp->m_paneProportions[1]);
                }
            } else if (pApp->m_draggingSplitter == 2) {
                float newSceneViewProportion =
                    pApp->m_paneProportions[1] + static_cast<float>(deltaX) / clientWidth;
                float newSceneTreeProportion =
                    pApp->m_paneProportions[2] - static_cast<float>(deltaX) / clientWidth;

                if (newSceneViewProportion > 0.1f && newSceneTreeProportion > 0.1f) {
                    pApp->m_paneProportions[1] = newSceneViewProportion;
                    pApp->m_paneProportions[2] = newSceneTreeProportion;
                    pApp->m_paneProportions[0] =
                        1.0f - (pApp->m_paneProportions[1] + pApp->m_paneProportions[2]);
                }
            }

            pApp->m_lastMouseX = currentMouseX;
            pApp->LayoutChildWindows(); // Call the non-static method on the instance
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        ReleaseCapture();
        pApp->m_draggingSplitter = 0;
        return 0;
    }
    case WM_SETCURSOR: {
        SetCursor(LoadCursorW(NULL, IDC_SIZEWE));
        return TRUE;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        HBRUSH hSplitterBrush = CreateSolidBrush(RGB(100, 100, 100));
        FillRect(hdc, &rc, hSplitterBrush);
        DeleteObject(hSplitterBrush);
        EndPaint(hwnd, &ps); // FIXED: Changed from EndPaint(hdc, &ps) to EndPaint(hwnd, &ps)
        return 0;
    }
    case WM_ERASEBKGND:
        return TRUE;

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

} // namespace Window