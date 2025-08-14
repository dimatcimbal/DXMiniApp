// src/Window/MainWindow.cpp
// Created by dtcimbal on 2/06/2025.
#include <Windows.h>  // Core Windows API functions (e.g., CreateWindowEx, DefWindowProc)
#include <CommCtrl.h> // Common Controls (e.g., InitCommonControlsEx, WC_TREEVIEW)
#include <stdexcept>  // For std::runtime_error, useful for more robust error handling
#include <string>     // For std::wstring and std::to_wstring (for debug output)

#include "FileView.h" // Include definitions for view component classes
#include "MainWindow.h"

#include <sstream>

#include "Common/Debug.h"
#include "Files/WorkingDirFileProvider.h"
#include "SceneTree.h"
#include "SceneView.h"

// Anonymous namespace for constants internal to this compilation unit
namespace {
const int SPLITTER_WIDTH = 5; // Thickness of the splitter bars in pixels
} // anonymous namespace

// Constructor: Initializes members and performs window class registration and main window creation.
MainWindow::MainWindow()
    : mHWnd(nullptr), mHwndSplitter1(nullptr), mHwndSplitter2(nullptr),
      mHInstance(GetModuleHandle(nullptr)) {
    // No pane proportions needed for fixed splitters; layout will be hardcoded.

    // Register the main window class. If registration fails, an error is logged.
    if (!RegisterWindowClass()) {
        DEBUG_ERROR(L"Failed to register main window class!\n");
        return;
    }

    // Create the main application window. If creation fails, an error is logged.
    if (!CreateMainWindow()) {
        DEBUG_ERROR(L"Failed to create main window!\n");
        return;
    }
}

// Destructor: Cleans up resources.
MainWindow::~MainWindow() {
    // No explicit cleanup for HWNDs or unique_ptrs needed here due to RAII and Windows' message
    // loop.
}

// Registers the window class for the main application window.
// Returns an ATOM that identifies the window class, or 0 on failure.
ATOM MainWindow::RegisterWindowClass() {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OnWindowMessage;             // Our static message handler
    wc.hInstance = mHInstance;                    // Instance handle for the application
    wc.lpszClassName = MAIN_CLASS_NAME;           // Unique class name
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW); // Standard arrow cursor
    wc.hbrBackground = nullptr;         // No default background erase; children will paint
    wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw on horizontal/vertical size changes

    ATOM atom = RegisterClassEx(&wc);
    if (atom == 0) {
        DEBUG_ERROR(L"RegisterClassEx failed for %s. Error: %s", MAIN_CLASS_NAME,
                    std::to_wstring(GetLastError()).c_str());
    }
    return atom;
}

// Creates the main application window.
// Returns true on success, false on failure.
bool MainWindow::CreateMainWindow() {
    // Verify that the main window class has been registered.
    WNDCLASSEX wcInfo = {};
    wcInfo.cbSize = sizeof(WNDCLASSEX);
    if (!GetClassInfoEx(mHInstance, MAIN_CLASS_NAME, &wcInfo)) {
        DEBUG_ERROR(L"CreateMainWindow: Class '%s' is NOT registered! Error: %s", MAIN_CLASS_NAME,
                    std::to_wstring(GetLastError()).c_str());
        return false;
    }

    mHWnd = CreateWindowEx(
        0,               // Optional window extended styles
        MAIN_CLASS_NAME, // Name of the registered window class
        L"DXMiniApp",    // Window title bar text
        WS_OVERLAPPEDWINDOW |
            WS_CLIPCHILDREN, // Standard overlapped window with clip children for child windows
        CW_USEDEFAULT, CW_USEDEFAULT,                // Default window position
        DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, // Default window size
        nullptr,                                     // No parent window (top-level window)
        nullptr,                                     // No menu
        mHInstance,                                  // Instance handle
        this // Pointer to 'this' passed as creation parameter (retrieved in WM_NCCREATE)
    );

    if (mHWnd == nullptr) {
        DEBUG_ERROR(L"CreateWindowEx failed. Error: %s", std::to_wstring(GetLastError()).c_str());
        return false;
    }

    // Display and update the main window.
    ShowWindow(mHWnd, SW_SHOWDEFAULT);
    UpdateWindow(mHWnd);

    return true;
}

// Static Window Procedure (Trampoline):
LRESULT CALLBACK MainWindow::OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Instance-specific message handler:
LRESULT MainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        if (!OnCreate(hWnd, reinterpret_cast<LPCREATESTRUCT>(lParam))) {
            DestroyWindow(hWnd);
            return -1; // Indicate failure to create the window
        }
        return 0;
    }
    case WM_COMMAND: {
        if (static_cast<UINT>(ChildWindowIDs::Exit) == LOWORD(wParam)) {
            DestroyWindow(hWnd);
        }
        return 0;
    }
    case WM_ERASEBKGND: {
        return TRUE;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_SIZE: {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        OnSize(width, height);
        return 0;
    }
    case WM_NOTIFY: {
        LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
        if (mFileView && lpnmh->hwndFrom == mFileView->GetHWND() && lpnmh->code == TVN_SELCHANGED) {
            // TODO Placeholder for file selection logic.
        }
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default: {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    }
}

// Handles the WM_CREATE message: This is where child view components and other UI elements are
// created.
bool MainWindow::OnCreate(HWND hWnd, LPCREATESTRUCT pcs) {
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // --- Create the Menu Bar ---
    HMENU hMenuBar = CreateMenu();
    if (hMenuBar == nullptr) {
        DEBUG_ERROR(L"Failed to create menu bar!\n");
        return false;
    }

    HMENU hSceneMenu = CreatePopupMenu();
    if (hSceneMenu == nullptr) {
        DEBUG_ERROR(L"Failed to create scene menu!\n");
        DestroyMenu(hMenuBar);
        return false;
    }

    AppendMenuW(hSceneMenu, MF_STRING, static_cast<UINT>(ChildWindowIDs::Exit), L"E&xit");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hSceneMenu), L"&Scene");
    SetMenu(hWnd, hMenuBar);

    // Create instances of our view components
    mFileProvider = std::make_unique<WorkingDirFileProvider>();

    mFileView = std::make_unique<FileView>(*mFileProvider);
    if (!mFileView->Create(hWnd, static_cast<UINT>(ChildWindowIDs::FileView))) {
        DEBUG_ERROR(L"Failed to create FileView!\n");
        return false;
    }

    mSceneView = std::make_unique<SceneView>();
    if (!mSceneView->Create(hWnd, static_cast<UINT>(ChildWindowIDs::SceneView))) {
        DEBUG_ERROR(L"Failed to create SceneView!\n");
        return false;
    }

    mSceneTree = std::make_unique<SceneTree>();
    if (!mSceneTree->Create(hWnd, static_cast<UINT>(ChildWindowIDs::SceneTree))) {
        DEBUG_ERROR(L"Failed to create SceneTree!\n");
        return false;
    }

    // Create the fixed splitter controls as simple static rectangles.
    // They are now purely visual separators and don't handle mouse input.
    HMENU splitter1Id = reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ChildWindowIDs::Splitter1));
    mHwndSplitter1 = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_GRAYRECT, 0, 0, 0,
                                    0, hWnd, splitter1Id, mHInstance, nullptr);
    if (!mHwndSplitter1) {
        DEBUG_ERROR(L"Failed to create m_hwndSplitter1!\n");
        return false;
    }

    HMENU splitter2Id = reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ChildWindowIDs::Splitter2));
    mHwndSplitter2 = CreateWindowEx(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_GRAYRECT, 0, 0, 0,
                                    0, hWnd, splitter2Id, mHInstance, nullptr);
    if (!mHwndSplitter2) {
        DEBUG_ERROR(L"Failed to create m_hwndSplitter2!\n");
        return false;
    }

    // Perform initial layout after all controls are created.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    OnSize(rcClient.right, rcClient.bottom); // This calls LayoutChildViews
    return true;
}

void MainWindow::Destroy() {
    // Destroy the child views. We have to get the raw pointers to deduce to BaseView.
    std::initializer_list<BaseView*> views = {mFileView.get(), mSceneTree.get(), mSceneView.get()};

    for (BaseView* view : views) {
        if (view) {
            view->Destroy();
        }
    }

    // Destroy the main window.
    if (mHWnd) {
        DestroyWindow(mHWnd);
        mHWnd = nullptr;
    }
}

int MainWindow::Run() {
    MSG msg = {};
    while (msg.message != WM_QUIT && OnUpdate()) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : 0;
}

bool MainWindow::OnUpdate() {
    // TODO Handle user input
    // TODO update the state/camera

    // Draw the scene
    if (mSceneView) {
        mSceneView->OnUpdate();
    }

    return true;
}

// Handles the WM_SIZE message to resize child views and update splitter positions.
void MainWindow::OnSize(int clientWidth, int clientHeight) {
    // Delegate to the layout helper function.
    LayoutChildViews(clientWidth, clientHeight);
}

// Helper to calculate and set the positions of all child views and splitters.
// This is called on WM_SIZE and also during splitter dragging.
void MainWindow::LayoutChildViews(int clientWidth, int clientHeight) {
    // The order here reflects the desired horizontal layout: FileView | Splitter1 | SceneView |
    // Splitter2 | SceneTree.

    // Define fixed proportions for the panes.
    const float fileViewProportion = 0.25f;  // 25% of width for FileView
    const float sceneViewProportion = 0.50f; // 50% of width for SceneView
    // The remaining for SceneTree

    int fileListWidth = static_cast<int>(clientWidth * fileViewProportion);
    int sceneViewWidth = static_cast<int>(clientWidth * sceneViewProportion);

    int xPos = 0; // Current X-position for placing the next control

    // Layout FileView (leftmost pane)
    if (mFileView && mFileView->GetHWND()) {
        SetWindowPos(mFileView->GetHWND(), nullptr, xPos, 0, fileListWidth, clientHeight,
                     SWP_NOZORDER);
    }
    xPos += fileListWidth;

    // Layout Splitter 1 (between FileView and SceneView)
    if (mHwndSplitter1) {
        SetWindowPos(mHwndSplitter1, nullptr, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    }
    xPos += SPLITTER_WIDTH;

    // Layout SceneView (middle pane)
    if (mSceneView && mSceneView->GetHWND()) {
        SetWindowPos(mSceneView->GetHWND(), nullptr, xPos, 0, sceneViewWidth, clientHeight,
                     SWP_NOZORDER);
    }
    xPos += sceneViewWidth;

    // Layout Splitter 2 (between SceneView and SceneTree)
    if (mHwndSplitter2) {
        SetWindowPos(mHwndSplitter2, nullptr, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    }
    xPos += SPLITTER_WIDTH;

    // Layout SceneTree (rightmost pane, takes up the remaining width)
    if (mSceneTree && mSceneTree->GetHWND()) {
        SetWindowPos(mSceneTree->GetHWND(), nullptr, xPos, 0, clientWidth - xPos, clientHeight,
                     SWP_NOZORDER);
    }
}
