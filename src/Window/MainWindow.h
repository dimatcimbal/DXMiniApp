// src/Window/MainWindow.h
#pragma once

#include <Windows.h>
#include <memory> // For std::unique_ptr
#include <string> // For std::wstring

#include "Util/BaseFileProvider.h"

// Forward declarations for view component classes
// This is a good practice to avoid circular dependencies and speed up compilation.
class FileView;
class SceneTree;
class SceneView;

// Define unique IDs for child windows and menu items using an enum class for strong typing.
// These IDs are shared between MainWindow and the views.
enum class ChildWindowIDs : UINT {
    Exit = 1000,
    FileView = 1001,
    SceneTree = 1002,
    SceneView = 1003,
    Splitter1 = 1004, // First vertical splitter (between FileView and SceneView)
    Splitter2 = 1005  // Second vertical splitter (between SceneView and SceneTree)
};

// Main window class name (used for registering the window class)
const static LPCWSTR MAIN_CLASS_NAME = L"DXMiniAppMainWindow";

// Default window dimensions
const static int DEFAULT_WINDOW_WIDTH = 1280;
const static int DEFAULT_WINDOW_HEIGHT = 720;

class MainWindow {
  public:
    // Constructor and Destructor
    MainWindow();
    ~MainWindow();

    // Accessor for the main window handle
    HWND GetHWND() const {
        return m_hWnd;
    }

    // --- Public members for SplitterProc access (declared public due to friend, could be private
    // with accessors) --- Last known mouse X position during a splitter drag operation
    int m_lastMouseX;
    // Tracks which splitter is being dragged (0: none, 1: splitter1, 2: splitter2)
    int m_draggingSplitter;
    // Handles for the splitter bar windows
    HWND m_hwndSplitter1;
    HWND m_hwndSplitter2;
    // Proportions of the three main panes (FileView, SceneView, SceneTree)
    // Normalized so their sum is typically 1.0f.
    float m_paneProportions[3];

    // --- Public helper methods (exposed for friend or if other parts of the app need to trigger
    // layout) --- Recalculates and sets the positions of all child views based on current window
    // size and pane proportions.
    void LayoutChildViews(int clientWidth, int clientHeight);

  private:
    // Main application window handle
    HWND m_hWnd;
    // Instance handle for the application, obtained during creation.
    HINSTANCE m_hInstance;

    // Smart pointers to manage the lifetime of our view components.
    std::unique_ptr<FileView> m_pFileView;
    std::unique_ptr<SceneTree> m_pSceneTree;
    std::unique_ptr<SceneView> m_pSceneView;
    std::unique_ptr<Util::BaseFileProvider> m_fileProvider;

    // --- Private helper methods for window management ---
    // Registers the window class for the main application window.
    ATOM RegisterWindowClass();
    // Registers a generic window class for child controls (e.g., custom splitters).
    ATOM RegisterChildWindowClass(LPCWSTR className,
                                  WNDPROC wndProc,
                                  HCURSOR hCursor,
                                  HBRUSH hbrBackground);
    // Creates the main application window after its class has been registered.
    bool CreateMainWindow();

    // --- Static and Instance-Specific Window Procedures ---
    // The static Window Procedure (trampoline) that dispatches messages to the correct MainWindow
    // instance.
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // The instance-specific message handler where most of the window's message processing logic
    // resides.
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // --- Message Handlers ---
    // Handles the WM_CREATE message, typically used for creating child windows and initializing UI.
    void OnCreate(HWND hWnd, LPCREATESTRUCT pcs);
    // Handles the WM_SIZE message, used for resizing and repositioning child windows.
    void OnSize(int clientWidth, int clientHeight);

    // Declare the global/static SplitterProc function as a friend to allow it to access
    // private/protected members of MainWindow (like m_lastMouseX, m_draggingSplitter,
    // m_paneProportions).
    friend LRESULT CALLBACK SplitterProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};