// src/Window/MainWindow.cpp
// Created by dtcimbal on 2/06/2025.
#include <Windows.h>  // Core Windows API functions (e.g., CreateWindowEx, DefWindowProc)
#include <CommCtrl.h> // Common Controls (e.g., InitCommonControlsEx, WC_TREEVIEW)
#include <stdexcept>  // For std::runtime_error, useful for more robust error handling
#include <string>     // For std::wstring and std::to_wstring (for debug output)

#include "FileView.h" // Include definitions for view component classes
#include "MainWindow.h"

#include <sstream>

#include "../Common/Debug.h"
#include "Common/Debug.h"
#include "Files/WorkingDirFileProvider.h"
#include "SceneTree.h"
#include "SceneView.h"

// Anonymous namespace for constants internal to this compilation unit
namespace {
const int SPLITTER_WIDTH = 5; // Thickness of the splitter bars in pixels
}

// Forward declaration for the global/static SplitterProc function.
// It's declared here because it needs to access MainWindow's members,
// and it's defined outside the class in this design pattern.
LRESULT CALLBACK SplitterProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Constructor: Initializes members and performs window class registration and main window creation.
MainWindow::MainWindow()
    : mHWnd(nullptr), mHwndSplitter1(nullptr), mHwndSplitter2(nullptr),
      mHInstance(GetModuleHandle(nullptr)), mDraggingSplitter(0), mLastMouseX(0) {
    // Initialize pane proportions for the three main sections (FileView, SceneView, SceneTree).
    // These values dictate the initial relative widths of the panes.
    mPaneProportions[0] = 0.33f; // FileView pane width proportion
    mPaneProportions[1] = 0.34f; // SceneView pane width proportion
    mPaneProportions[2] = 0.33f; // SceneTree pane width proportion

    // Register the main window class. If registration fails, an error is logged.
    if (!RegisterWindowClass()) {
        DEBUGPRINT(L"ERROR: Failed to register main window class!\n");
        // In a real application, you might throw an exception or exit here.
        return;
    }

    // Create the main application window. If creation fails, an error is logged.
    if (!CreateMainWindow()) {
        DEBUGPRINT(L"ERROR: Failed to create main window!\n");
        // In a real application, you might throw an exception or exit here.
        return;
    }
}

// Destructor: Cleans up resources.
// unique_ptr members automatically handle deletion of managed objects.
// HWNDs are automatically destroyed by Windows when the parent window is destroyed (WM_DESTROY).
MainWindow::~MainWindow() {
    // No explicit cleanup for HWNDs or unique_ptrs needed here due to RAII and Windows' message
    // loop.
}

// Registers the window class for the main application window.
// Returns an ATOM that identifies the window class, or 0 on failure.
ATOM MainWindow::RegisterWindowClass() {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = StaticWindowProc;            // Our static message handler
    wc.hInstance = mHInstance;                    // Instance handle for the application
    wc.lpszClassName = MAIN_CLASS_NAME;           // Unique class name
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW); // Standard arrow cursor
    wc.hbrBackground = nullptr;         // No default background erase; children will paint
    wc.style = CS_HREDRAW | CS_VREDRAW; // Redraw on horizontal/vertical size changes

    ATOM atom = RegisterClassEx(&wc);
    if (atom == 0) {
        DEBUGPRINT(L"RegisterClassEx failed for %s. Error: %s", MAIN_CLASS_NAME,
                   std::to_wstring(GetLastError()).c_str());
    }
    return atom;
}

// Registers a generic window class for child controls (e.g., custom splitter bars).
// Returns an ATOM that identifies the window class, or 0 on failure.
ATOM MainWindow::RegisterChildWindowClass(LPCWSTR className,
                                          WNDPROC wndProc,
                                          HCURSOR hCursor,
                                          HBRUSH hbrBackground) {
    WNDCLASSEX wc{};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = wndProc;
    wc.hInstance = mHInstance;
    wc.lpszClassName = className;
    wc.hCursor = hCursor;
    wc.hbrBackground = hbrBackground; // Can be a solid brush for splitters

    ATOM atom = RegisterClassEx(&wc);
    if (atom == 0) {
        DEBUGPRINT(L"RegisterClassEx failed for child class %s. Error: %s", className,
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
        DEBUGPRINT(L"CreateMainWindow: Class '%s' is NOT registered! Error: %s", MAIN_CLASS_NAME,
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
        DEBUGPRINT((L"CreateWindowEx failed. Error: %s", std::to_wstring(GetLastError())).c_str());
        return false;
    }

    // Display and update the main window.
    ShowWindow(mHWnd, SW_SHOWDEFAULT);
    UpdateWindow(mHWnd);

    return true;
}

// Static Window Procedure (Trampoline):
// This static function is the initial entry point for Windows messages.
// It retrieves the 'this' pointer of the MainWindow instance and then
// dispatches the message to the instance-specific HandleMessage method.
LRESULT CALLBACK MainWindow::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        // On WM_NCCREATE, the CREATESTRUCT contains the 'this' pointer in lpCreateParams.
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = static_cast<MainWindow*>(pCreate->lpCreateParams);
        // Store the 'this' pointer in the window's user data for future messages.
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        // For all other messages, retrieve the 'this' pointer from the window's user data.
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    // If a valid instance pointer is found, let the instance handle the message.
    if (pThis) {
        return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
    }

    // If no instance pointer (or if the message is handled before WM_NCCREATE),
    // default processing is performed.
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// Instance-specific message handler:
// Contains the main logic for processing Windows messages for this MainWindow instance.
LRESULT MainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Calls the OnCreate handler to set up child windows and UI.
        OnCreate(hWnd, reinterpret_cast<LPCREATESTRUCT>(lParam));
        return 0; // Message handled
    }
    case WM_COMMAND: {
        // Handles commands from menus, accelerators, or controls.
        switch (LOWORD(wParam)) {
        case static_cast<UINT>(ChildWindowIDs::Exit): // Use enum class ID
            DestroyWindow(hWnd);                      // Close the main window
            break;
        }
        return 0; // Message handled
    }
    case WM_ERASEBKGND: {
        // Return TRUE to indicate that the background is erased, preventing flicker.
        // Child windows will handle their own painting.
        return TRUE;
    }
    case WM_PAINT: {
        // Standard painting boilerplate. In this app, actual rendering is done by SceneView.
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0; // Message handled
    }
    case WM_SIZE: {
        // Handles window resizing, triggering layout recalculation for child views.
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        OnSize(width, height);
        return 0; // Message handled
    }
    case WM_NOTIFY: {
        // Handles notifications from common controls (e.g., TreeView selection changes).
        LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
        if (mFileView && lpnmh->hwndFrom == mFileView->GetHWND() && lpnmh->code == TVN_SELCHANGED) {
            // Placeholder for file selection logic.
            // You would typically call a method on FileView or pass parameters for
            // SceneView/SceneTree update. Example:
            // m_pFileView->HandleSelectionChange(reinterpret_cast<LPNMTREEVIEW>(lParam),
            // m_pSceneView->GetHWND(), m_pSceneTree->GetHWND());
        }
        return 0; // Message handled
    }
    case WM_DESTROY: {
        // Posts a quit message to the message loop, signaling the application to exit.
        PostQuitMessage(0);
        return 0; // Message handled
    }
    default: {
        // For any messages not explicitly handled, pass to the default window procedure.
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    }
}

// Handles the WM_CREATE message: This is where child view components and other UI elements are
// created.
void MainWindow::OnCreate(HWND hWnd, LPCREATESTRUCT pcs) {
    // Initialize common controls library for TreeView, ListBox, etc.
    INITCOMMONCONTROLSEX icex{};
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // --- Create the Menu Bar ---
    HMENU hMenuBar = CreateMenu();
    if (hMenuBar == nullptr) {
        DEBUGPRINT(L"ERROR: Failed to create menu bar!\n");
        return;
    }

    HMENU hSceneMenu = CreatePopupMenu();
    if (hSceneMenu == nullptr) {
        DEBUGPRINT(L"ERROR: Failed to create scene menu!\n");
        DestroyMenu(hMenuBar); // Clean up
        return;
    }

    AppendMenuW(hSceneMenu, MF_STRING, static_cast<UINT>(ChildWindowIDs::Exit), L"E&xit");
    AppendMenuW(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hSceneMenu), L"&Scene");
    SetMenu(hWnd, hMenuBar);

    // Register the custom "SplitterWindow" class.
    HCURSOR resizeCursor = LoadCursorW(nullptr, IDC_SIZEWE);
    HBRUSH splitterBrush = CreateSolidBrush(RGB(100, 100, 100));
    if (!RegisterChildWindowClass(L"SplitterWindow", SplitterProc, resizeCursor, splitterBrush)) {
        DEBUGPRINT(L"ERROR: Failed to register SplitterWindow class!\n");
        DeleteObject(splitterBrush); // Clean up brush
        return;
    }
    DeleteObject(splitterBrush); // Clean up brush after successful registration

    // Create instances of our view components using unique_ptr for automatic memory management.
    // and create the actual Windows controls for each view.
    mFileProvider = std::make_unique<WorkingDirFileProvider>();

    mFileView = std::make_unique<FileView>(*mFileProvider);
    if (mFileView)
        mFileView->Create(hWnd, static_cast<UINT>(ChildWindowIDs::FileView));

    mSceneView = std::make_unique<SceneView>();
    if (mSceneView)
        mSceneView->Create(hWnd, static_cast<UINT>(ChildWindowIDs::SceneView));

    mSceneTree = std::make_unique<SceneTree>();
    if (mSceneTree)
        mSceneTree->Create(hWnd, static_cast<UINT>(ChildWindowIDs::SceneTree));

    // Create the splitter controls.
    HMENU splitter1Id = reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ChildWindowIDs::Splitter1));
    mHwndSplitter1 = CreateWindowEx(0, L"SplitterWindow", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                    hWnd, splitter1Id, mHInstance, nullptr);
    if (!mHwndSplitter1)
        DEBUGPRINT(L"ERROR: Failed to create m_hwndSplitter1!\n");

    HMENU splitter2Id = reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ChildWindowIDs::Splitter2));
    mHwndSplitter2 = CreateWindowEx(0, L"SplitterWindow", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0,
                                    hWnd, splitter2Id, mHInstance, nullptr);
    if (!mHwndSplitter2)
        DEBUGPRINT(L"ERROR: Failed to create m_hwndSplitter2!\n");

    // Perform initial layout after all controls are created.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    OnSize(rcClient.right, rcClient.bottom); // This calls LayoutChildViews
}

int MainWindow::Run() {
    MSG msg = {};
    // Loop as long as the application's OnUpdate() method says to continue
    // AND as long as WM_QUIT has not been received.
    // WM_QUIT takes precedence for a clean shutdown.
    while (msg.message != WM_QUIT && OnUpdate()) {
        // Process all pending Windows messages without blocking
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break; // Exit the inner loop immediately if WM_QUIT is found
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Ensure WM_QUIT's wParam is returned. If we exited due to OnUpdate() returning false
    // *before* WM_QUIT was processed, msg.wParam might be 0.
    // A robust way: if WM_QUIT was explicitly received, use its wParam. Otherwise, default to 0.
    return (msg.message == WM_QUIT) ? static_cast<int>(msg.wParam) : 0;
}

bool MainWindow::OnUpdate() {
    // TODO Handle user input
    // TODO update the state/camera

    // Draw the scene
    if (mSceneView) {
        mSceneView->RenderScene(mCamera);
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

    // Calculate initial pane widths based on proportions.
    // clientWidth is the total usable width for all panes and splitters.
    int fileListWidth = static_cast<int>(clientWidth * mPaneProportions[0]);
    int sceneViewWidth = static_cast<int>(clientWidth * mPaneProportions[1]);
    // SceneTree width will be the remaining space after FileView, Splitter1, SceneView, and
    // Splitter2.

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

// Global/Static Splitter Window Procedure:
// This function handles messages specifically for the splitter bar windows.
// It retrieves the parent MainWindow instance to update its pane proportions
// and trigger a layout refresh.
LRESULT CALLBACK SplitterProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Get the MainWindow instance associated with the parent window of the splitter.
    HWND hwndParent = GetParent(hwnd);
    MainWindow* pMainWindow =
        reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwndParent, GWLP_USERDATA));

    // If the MainWindow instance isn't found (shouldn't happen in a well-behaved app),
    // fall back to default processing.
    if (!pMainWindow) {
        DEBUGPRINT(L"ERROR: SplitterProc failed to retrieve MainWindow instance!\n");
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        // When the left mouse button is pressed on a splitter, capture the mouse
        // to receive all subsequent mouse messages, even if the cursor leaves the splitter.
        SetCapture(hwnd);
        pMainWindow->mLastMouseX = LOWORD(lParam); // Store the initial mouse X position.

        // Determine which splitter is being dragged based on its HWND.
        if (hwnd == pMainWindow->mHwndSplitter1) {
            pMainWindow->mDraggingSplitter = 1;
        } else if (hwnd == pMainWindow->mHwndSplitter2) {
            pMainWindow->mDraggingSplitter = 2;
        }
        return 0; // Message handled
    }
    case WM_MOUSEMOVE: {
        bool isDragging = (pMainWindow->mDraggingSplitter != 0);
        bool isLeftButtonDown = (wParam & MK_LBUTTON);

        if (isDragging && isLeftButtonDown) {
            const int currentMouseX = LOWORD(lParam);
            const int deltaX = currentMouseX - pMainWindow->mLastMouseX;

            RECT rcClient;
            GetClientRect(hwndParent, &rcClient);
            const int clientWidth = rcClient.right - rcClient.left;
            const int clientHeight = rcClient.bottom - rcClient.top;

            if (clientWidth == 0)
                return 0;

            if (pMainWindow->mDraggingSplitter == 1) { // Dragging Splitter1
                float mouseDeltaRatio = static_cast<float>(deltaX) / clientWidth;
                float newFileViewProportion = pMainWindow->mPaneProportions[0] + mouseDeltaRatio;
                float newSceneViewProportion = pMainWindow->mPaneProportions[1] - mouseDeltaRatio;

                const float MIN_PANE_PROPORTION = 0.1f; // 10% minimum width

                if (newFileViewProportion > MIN_PANE_PROPORTION &&
                    newSceneViewProportion > MIN_PANE_PROPORTION) {
                    pMainWindow->mPaneProportions[0] = newFileViewProportion;
                    pMainWindow->mPaneProportions[1] = newSceneViewProportion;
                    // Recalculate the third pane's proportion to ensure the sum remains 1.0f.
                    pMainWindow->mPaneProportions[2] = 1.0f - (pMainWindow->mPaneProportions[0] +
                                                               pMainWindow->mPaneProportions[1]);
                }
            } else if (pMainWindow->mDraggingSplitter == 2) { // Dragging Splitter2
                float mouseDeltaRatio = static_cast<float>(deltaX) / clientWidth;
                float newSceneViewProportion = pMainWindow->mPaneProportions[1] + mouseDeltaRatio;
                float newSceneTreeProportion = pMainWindow->mPaneProportions[2] - mouseDeltaRatio;

                const float MIN_PANE_PROPORTION = 0.1f; // 10% minimum width

                if (newSceneViewProportion > MIN_PANE_PROPORTION &&
                    newSceneTreeProportion > MIN_PANE_PROPORTION) {
                    pMainWindow->mPaneProportions[1] = newSceneViewProportion;
                    pMainWindow->mPaneProportions[2] = newSceneTreeProportion;
                    pMainWindow->mPaneProportions[0] = 1.0f - (pMainWindow->mPaneProportions[1] +
                                                               pMainWindow->mPaneProportions[2]);
                }
            }

            pMainWindow->mLastMouseX = currentMouseX;
            pMainWindow->LayoutChildViews(clientWidth, clientHeight);

            InvalidateRect(hwndParent, nullptr, TRUE);
            UpdateWindow(hwndParent);
        }
        return 0; // Message handled
    }
    case WM_LBUTTONUP: {
        // When the left mouse button is released, release mouse capture and reset dragging state.
        ReleaseCapture();
        pMainWindow->mDraggingSplitter = 0;
        return 0; // Message handled
    }
    case WM_SETCURSOR: {
        // Set the cursor to a horizontal resize cursor when the mouse is over the splitter.
        SetCursor(LoadCursorW(nullptr, IDC_SIZEWE));
        return TRUE; // Indicate that we set the cursor
    }
    case WM_PAINT: {
        // Custom painting for the splitter bar (e.g., to draw its grey background).
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        // Create a solid grey brush for the splitter.
        HBRUSH hSplitterBrush = CreateSolidBrush(RGB(100, 100, 100));
        FillRect(hdc, &rc, hSplitterBrush);
        DeleteObject(hSplitterBrush); // Clean up the brush resource.
        EndPaint(hwnd, &ps);
        return 0; // Message handled
    }
    case WM_ERASEBKGND: {
        // Return TRUE to prevent default background erasing and reduce flicker.
        // The WM_PAINT handler will draw the background.
        return TRUE;
    }
    default:
        // For any other messages, pass to the default window procedure.
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}