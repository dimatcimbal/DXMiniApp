// Explicitly define UNICODE and _UNICODE at the top to force wide character APIs
#define UNICODE
#define _UNICODE

// Remove tchar.h as we are explicitly going for wide characters
// #include <tchar.h>
#include <windows.h>
#include <gdiplus.h>
#include <CommCtrl.h> // For TreeView and ListBox controls
#include <Shlwapi.h>  // For PathCombine, PathFindFileName
// #include <strsafe.h>  // For StringCchPrintf - often has TCHAR variants, may not be strictly
// needed here if we use std::wstring

// C++17 includes
#include <filesystem> // For file system operations
#include <memory>     // For std::unique_ptr
#include <stdexcept>  // For std::runtime_error (for better error handling if desired)
#include <string>
#include <vector>

// Link with these libraries
#pragma comment(lib, "Gdiplus.lib")
#pragma comment(lib, "Comctl32.lib") // For common controls
#pragma comment(lib, "Shlwapi.lib")  // For PathCombine, PathFindFileName

// Import GDI+ namespace
using namespace Gdiplus;

// --- Global Handles for Child Windows ---
HWND g_hwndFileList = NULL;
HWND g_hwndDirectXView = NULL;
HWND g_hwndSceneTree = NULL;
HWND g_hwndSplitter1 = NULL; // Between FileList and DirectXView
HWND g_hwndSplitter2 = NULL; // Between DirectXView and SceneTree

// --- Custom Window Messages ---
#define WM_APP_FILE_SELECTED (WM_APP + 1)

// --- Constants ---
constexpr int SPLITTER_WIDTH = 6; // Width of the draggable splitter bar

// --- Global Variables for Resizing ---
// Store the current desired widths of the three content panes
// These will be percentages or proportions of the total client width
float g_paneProportions[3] = {0.33f, 0.34f,
                              0.33f}; // Proportions for FileList, DirectXView, SceneTree
int g_draggingSplitter = 0;           // 0: no drag, 1: dragging splitter1, 2: dragging splitter2
int g_lastMouseX = 0;                 // Last mouse X position during a drag

// --- GDI+ initialization and shutdown (remains in WinMain) ---
GdiplusStartupInput gdiplusStartupInput;
ULONG_PTR gdiplusToken;

// --- Function Prototypes ---
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DirectXViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SplitterProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Helper function to layout child windows (now takes const HWND)
void LayoutChildWindows(HWND hwndParent);

// Updated function to populate file list using std::filesystem
void PopulateFileList(HWND hListBox);

// Updated function to load scene into tree (takes std::wstring)
void LoadSceneIntoTree(HWND hTreeView, const std::wstring& filePath);

// Helper for registering window classes
ATOM RegisterChildWindowClass(const WNDCLASSW& wc);

// --- Helper function to lay out child windows ---
void LayoutChildWindows(HWND hwndParent) {
    RECT rcClient;
    GetClientRect(hwndParent, &rcClient);
    const int clientWidth = rcClient.right - rcClient.left;
    const int clientHeight = rcClient.bottom - rcClient.top;

    // Calculate actual pixel widths based on proportions
    int fileListWidth = static_cast<int>(clientWidth * g_paneProportions[0]);
    int directXViewWidth = static_cast<int>(clientWidth * g_paneProportions[1]);

    int xPos = 0;

    // File List
    SetWindowPos(g_hwndFileList, NULL, xPos, 0, fileListWidth, clientHeight, SWP_NOZORDER);
    xPos += fileListWidth;

    // Splitter 1
    SetWindowPos(g_hwndSplitter1, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    xPos += SPLITTER_WIDTH;

    // DirectX View
    SetWindowPos(g_hwndDirectXView, NULL, xPos, 0, directXViewWidth, clientHeight, SWP_NOZORDER);
    xPos += directXViewWidth;

    // Splitter 2
    SetWindowPos(g_hwndSplitter2, NULL, xPos, 0, SPLITTER_WIDTH, clientHeight, SWP_NOZORDER);
    xPos += SPLITTER_WIDTH;

    // Scene Tree (fill remaining space to avoid gaps due to rounding)
    SetWindowPos(g_hwndSceneTree, NULL, xPos, 0, clientWidth - xPos, clientHeight, SWP_NOZORDER);

    // Invalidate child windows to ensure they repaint with new sizes
    InvalidateRect(g_hwndFileList, NULL, TRUE);
    InvalidateRect(g_hwndDirectXView, NULL, TRUE);
    InvalidateRect(g_hwndSceneTree, NULL, TRUE);
    InvalidateRect(g_hwndSplitter1, NULL, TRUE);
    InvalidateRect(g_hwndSplitter2, NULL, TRUE);
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
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        // Initialize common controls (for TreeView and ListBox)
        INITCOMMONCONTROLSEX icex{}; // C++11 aggregate initialization
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        // ICC_STANDARD_CLASSES covers basic controls like ListBox.
        icex.dwICC = ICC_TREEVIEW_CLASSES | ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        // --- Register Child Window Classes ---
        WNDCLASSW wcSplitter{}; // Use WNDCLASSW
        wcSplitter.lpfnWndProc = SplitterProc;
        wcSplitter.hInstance = GetModuleHandle(NULL);
        wcSplitter.lpszClassName = L"SplitterWindow";
        wcSplitter.hCursor = LoadCursorW(NULL, IDC_SIZEWE);
        wcSplitter.hbrBackground = CreateSolidBrush(RGB(100, 100, 100)); // Dark gray splitter bar
        RegisterChildWindowClass(wcSplitter);

        WNDCLASSW wcDirectX{};
        wcDirectX.lpfnWndProc = DirectXViewProc;
        wcDirectX.hInstance = GetModuleHandle(NULL);
        wcDirectX.lpszClassName = L"DirectXViewWindow";
        wcDirectX.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1); // A light background for placeholder
        RegisterChildWindowClass(wcDirectX);

        // --- Create Child Windows ---
        g_hwndFileList = CreateWindowExW(WS_EX_CLIENTEDGE,
                                         WC_LISTBOX, // List box class name
                                         NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 0,
                                         0, 0, 0, hwnd, (HMENU)1001, GetModuleHandle(NULL), NULL);
        if (g_hwndFileList == NULL)
            return -1;
        PopulateFileList(g_hwndFileList);

        g_hwndSplitter1 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1004, GetModuleHandle(NULL), NULL);
        if (g_hwndSplitter1 == NULL)
            return -1;

        g_hwndDirectXView =
            CreateWindowExW(WS_EX_CLIENTEDGE, L"DirectXViewWindow", NULL, WS_CHILD | WS_VISIBLE, 0,
                            0, 0, 0, hwnd, (HMENU)1002, GetModuleHandle(NULL), NULL);
        if (g_hwndDirectXView == NULL)
            return -1;

        g_hwndSplitter2 = CreateWindowExW(0, L"SplitterWindow", NULL, WS_CHILD | WS_VISIBLE, 0, 0,
                                          0, 0, hwnd, (HMENU)1005, GetModuleHandle(NULL), NULL);
        if (g_hwndSplitter2 == NULL)
            return -1;

        g_hwndSceneTree =
            CreateWindowExW(WS_EX_CLIENTEDGE,
                            WC_TREEVIEW, // Tree view class name
                            NULL,
                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | TVS_HASBUTTONS |
                                TVS_HASLINES | TVS_LINESATROOT,
                            0, 0, 0, 0, hwnd, (HMENU)1003, GetModuleHandle(NULL), NULL);
        if (g_hwndSceneTree == NULL)
            return -1;

        // Initial layout
        LayoutChildWindows(hwnd);
        return 0;
    }

    case WM_SIZE: {
        [[maybe_unused]] int clientWidth = LOWORD(lParam);
        [[maybe_unused]] int clientHeight = HIWORD(lParam);
        LayoutChildWindows(hwnd);
        return 0;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case 1001: // File List Box ID
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                const int selectedIndex =
                    static_cast<int>(SendMessageW(g_hwndFileList, LB_GETCURSEL, 0, 0));
                if (selectedIndex != LB_ERR) {
                    wchar_t szFilePathBuffer[MAX_PATH];
                    SendMessageW(g_hwndFileList, LB_GETTEXT, selectedIndex,
                                 (LPARAM)szFilePathBuffer);
                    std::wstring selectedFileName = szFilePathBuffer;

                    std::filesystem::path currentPath = std::filesystem::current_path();
                    std::filesystem::path fullPath = currentPath / selectedFileName;
                    std::wstring fullPathStr = fullPath.native(); // Get native path string

                    // Send message with full path
                    SendMessageW(g_hwndDirectXView, WM_APP_FILE_SELECTED, 0,
                                 (LPARAM)fullPathStr.c_str());
                    SendMessageW(g_hwndSceneTree, WM_APP_FILE_SELECTED, 0,
                                 (LPARAM)fullPathStr.c_str());

                    // Use OutputDebugStringW for wide character strings
                    OutputDebugStringW((L"Selected File: " + fullPathStr + L"\n").c_str());
                }
            }
            break;
        }
        return 0;
    }

    case WM_DESTROY: {
        // Clean up brushes created for splitter backgrounds
        HBRUSH hBrush = (HBRUSH)GetClassLongPtrW(g_hwndSplitter1, GCLP_HBRBACKGROUND);
        if (hBrush) {
            DeleteObject(hBrush);
        }
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// Window procedure for the DirectX View child window
LRESULT CALLBACK DirectXViewProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Use std::unique_ptr for automatic cleanup of Gdiplus::Graphics
    static std::unique_ptr<Gdiplus::Graphics> graphics;
    static std::wstring s_currentSceneFile; // Use std::wstring for storing the path

    switch (uMsg) {
    case WM_CREATE: {
        HDC hdc = GetDC(hwnd);
        if (hdc) {
            graphics = std::make_unique<Gdiplus::Graphics>(hdc);
            graphics->SetTextRenderingHint(TextRenderingHintAntiAlias);
            ReleaseDC(hwnd, hdc);
        } else {
            OutputDebugStringW(L"DirectX View: Failed to get HDC in WM_CREATE!\n");
        }

        // --- DirectX Initialization Placeholder ---
        OutputDebugStringW(L"DirectX View: WM_CREATE - Placeholder for DirectX Init\n");
        return 0;
    }

    case WM_SIZE: {
        // When the DirectX view window resizes, you'll need to resize your DirectX swap chain
        // buffers and potentially update projection matrices.
        OutputDebugStringW(L"DirectX View: WM_SIZE - Placeholder for DirectX Resize\n");
        InvalidateRect(hwnd, NULL, TRUE); // Force repaint on resize
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        if (graphics) {
            // Re-associate graphics object with current HDC if needed
            // This is generally handled by GDI+ internally, but explicit re-creation for
            // robustness:
            graphics.reset();                                    // Destroy existing
            graphics = std::make_unique<Gdiplus::Graphics>(hdc); // Re-create from current HDC

            graphics->Clear(Color(255, 60, 60, 60));         // Dark gray background
            SolidBrush textBrush(Color(255, 255, 255, 255)); // White text
            Font font(L"Arial", 20, FontStyleRegular, UnitPixel);

            if (!s_currentSceneFile.empty()) {
                std::wstring displayString = L"Rendering: ";
                displayString += std::filesystem::path(s_currentSceneFile)
                                     .filename()
                                     .native(); // Use filesystem for filename
                displayString += L" (DirectX View)";
                graphics->DrawString(displayString.c_str(), -1, &font, PointF(10.0f, 10.0f),
                                     &textBrush);
            } else {
                graphics->DrawString(L"DirectX View (No Scene Loaded)", -1, &font,
                                     PointF(10.0f, 10.0f), &textBrush);
            }

            // --- Placeholder for DirectX Rendering ---
            // Example:
            // MyDirectXRenderScene(s_currentSceneFile);
            // MyDirectXPresent();
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_APP_FILE_SELECTED: {
        // Correctly assign to std::wstring using reinterpret_cast for wide characters
        s_currentSceneFile = reinterpret_cast<const wchar_t *>(lParam);
        InvalidateRect(hwnd, NULL, TRUE); // Force a repaint
        OutputDebugStringW(
            (L"DirectX View: Received WM_APP_FILE_SELECTED: " + s_currentSceneFile + L"\n")
                .c_str());
        return 0;
    }

    case WM_DESTROY: {
        graphics.reset(); // Release GDI+ Graphics object
        // --- DirectX Shutdown Placeholder ---
        OutputDebugStringW(L"DirectX View: WM_DESTROY - Placeholder for DirectX Shutdown\n");
        return 0;
    }

    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// Window procedure for the Splitter child windows
LRESULT CALLBACK SplitterProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_LBUTTONDOWN: {
        SetCapture(hwnd);
        g_lastMouseX = LOWORD(lParam);
        if (hwnd == g_hwndSplitter1) {
            g_draggingSplitter = 1;
        } else if (hwnd == g_hwndSplitter2) {
            g_draggingSplitter = 2;
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (g_draggingSplitter != 0 && (wParam & MK_LBUTTON)) {
            const int currentMouseX = LOWORD(lParam);
            const int deltaX = currentMouseX - g_lastMouseX;

            RECT rcClient;
            GetClientRect(GetParent(hwnd), &rcClient);
            const int clientWidth = rcClient.right - rcClient.left;

            if (clientWidth == 0)
                return 0; // Avoid division by zero

            if (g_draggingSplitter == 1) { // Splitter between FileList and DirectXView
                float newFileListProportion =
                    g_paneProportions[0] + static_cast<float>(deltaX) / clientWidth;
                float newDirectXViewProportion =
                    g_paneProportions[1] - static_cast<float>(deltaX) / clientWidth;

                // Basic bounds checking (e.g., min 10% width)
                if (newFileListProportion > 0.1f && newDirectXViewProportion > 0.1f) {
                    g_paneProportions[0] = newFileListProportion;
                    g_paneProportions[1] = newDirectXViewProportion;
                    g_paneProportions[2] =
                        1.0f - (g_paneProportions[0] + g_paneProportions[1]); // Ensure total is 1.0
                }
            } else if (g_draggingSplitter == 2) { // Splitter between DirectXView and SceneTree
                float newDirectXViewProportion =
                    g_paneProportions[1] + static_cast<float>(deltaX) / clientWidth;
                float newSceneTreeProportion =
                    g_paneProportions[2] - static_cast<float>(deltaX) / clientWidth;

                if (newDirectXViewProportion > 0.1f && newSceneTreeProportion > 0.1f) {
                    g_paneProportions[1] = newDirectXViewProportion;
                    g_paneProportions[2] = newSceneTreeProportion;
                    g_paneProportions[0] =
                        1.0f - (g_paneProportions[1] + g_paneProportions[2]); // Ensure total is 1.0
                }
            }

            g_lastMouseX = currentMouseX;
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
    default:
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
}

// Function to populate the file list box with files from the current directory using
// std::filesystem
void PopulateFileList(HWND hListBox) {
    try {
        std::filesystem::path currentDir = std::filesystem::current_path();
        for (const auto& entry : std::filesystem::directory_iterator(currentDir)) {
            if (entry.is_regular_file()) { // Only add regular files
                // SendMessage expects LPCTSTR, which resolves to const char* or const wchar_t*
                // Since _T is used, it will be wchar_t* in Unicode builds, matching
                // std::filesystem::path::c_str() native()
                SendMessageW(hListBox, LB_ADDSTRING, 0, (LPARAM)entry.path().filename().c_str());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::wstring errorMsg = L"Filesystem error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, NULL, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    } catch (const std::exception& e) {
        std::wstring errorMsg = L"General error: ";
        // Convert char* to wstring for MessageBoxW
        std::string narrowMsg = e.what();
        int bufferSize = MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, NULL, 0);
        if (bufferSize > 0) {
            std::vector<wchar_t> wBuf(bufferSize);
            MultiByteToWideChar(CP_ACP, 0, narrowMsg.c_str(), -1, wBuf.data(), bufferSize);
            errorMsg += wBuf.data();
        }
        MessageBoxW(NULL, errorMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
    }
}

// Function to load a 3D scene structure into the tree view
void LoadSceneIntoTree(HWND hTreeView, const std::wstring& filePath) {
    TreeView_DeleteAllItems(hTreeView); // Clear existing items

    OutputDebugStringW((L"Scene Tree: Received WM_APP_FILE_SELECTED: " + filePath + L"\n").c_str());

    HTREEITEM hRoot = NULL;
    TVINSERTSTRUCTW tvInsert{}; // C++11 aggregate initialization
    tvInsert.hParent = TVI_ROOT;
    tvInsert.hInsertAfter = TVI_LAST;
    tvInsert.item.mask = TVIF_TEXT;

    // Use std::filesystem to get just the filename
    std::wstring fileName = std::filesystem::path(filePath).filename().native();
    // const_cast is necessary here as TVINSERTSTRUCT::item.pszText is LPWSTR (non-const)
    tvInsert.item.pszText = const_cast<LPWSTR>(fileName.c_str());

    hRoot = TreeView_InsertItem(hTreeView, &tvInsert);

    if (hRoot) {
        tvInsert.hParent = hRoot;
        // Explicitly use L"" for string literals and const_cast to LPWSTR
        tvInsert.item.pszText = const_cast<LPWSTR>(L"Scene Root Node");
        HTREEITEM hSceneRoot = TreeView_InsertItem(hTreeView, &tvInsert);

        if (hSceneRoot) {
            tvInsert.hParent = hSceneRoot;
            tvInsert.item.pszText = const_cast<LPWSTR>(L"Mesh 1: Cube");
            TreeView_InsertItem(hTreeView, &tvInsert);

            tvInsert.item.pszText = const_cast<LPWSTR>(L"Mesh 2: Sphere");
            TreeView_InsertItem(hTreeView, &tvInsert);

            tvInsert.item.pszText = const_cast<LPWSTR>(L"Light 1: Directional");
            TreeView_InsertItem(hTreeView, &tvInsert);

            HTREEITEM hCamera = TreeView_InsertItem(hTreeView, &tvInsert);
            if (hCamera) {
                tvInsert.hParent = hCamera;
                tvInsert.item.pszText = const_cast<LPWSTR>(L"Camera Properties");
                TreeView_InsertItem(hTreeView, &tvInsert);
            }
        }
        // Expand the root node for better visibility
        TreeView_Expand(hTreeView, hRoot, TVE_EXPAND);
    }
}

int WINAPI WinMain(HINSTANCE hInstance,
                   [[maybe_unused]] HINSTANCE hPrevInstance,
                   [[maybe_unused]] LPSTR lpCmdLine,
                   int nCmdShow) {
    // GDI+ initialization
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    const WCHAR MAIN_CLASS_NAME[] = L"DXMiniAppWindow";

    WNDCLASSW wc{}; // C++11 aggregate initialization
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = MAIN_CLASS_NAME;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Failed to register main window class!", L"Error", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        return 0;
    }

    HWND hwnd = CreateWindowExW(0, MAIN_CLASS_NAME,
                                L"DXMiniApp - C++17 Resizable Panes", // Use L"" here
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700, NULL,
                                NULL, hInstance, NULL);

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