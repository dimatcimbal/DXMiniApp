// src/Window/MainWindow.h
// Created by dtcimbal on 2/06/2025.
#pragma once

#include <memory> // For std::unique_ptr
#include <string> // For std::wstring
#include "Includes/WindowsInclude.h"

#include "Files/BaseFileProvider.h"
#include "Scene/Camera.h"

#include <Graphics/GraphicsContext.h>
#include "Window/FileView.h"
#include "Window/SceneTree.h"
#include "Window/SceneView.h"

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
    // Static factory method to create and initialize a MainWindow instance.
    static bool MainWindow::Create(std::unique_ptr<MainWindow>& OutWindow);

    // Constructor/Destructor
    MainWindow()
        : mHWnd(nullptr), mHwndSplitter1(nullptr), mHwndSplitter2(nullptr),
          mHInstance(GetModuleHandle(nullptr)) {
    }
    ~MainWindow() = default;

    // --- Public helper methods (exposed for friend or if other parts of the app need to trigger
    // layout) --- Recalculates and sets the positions of all child views based on current window
    // size and pane proportions.
    void LayoutChildViews(int clientWidth, int clientHeight);

    // Destroys the main window and cleans up resources.
    void Destroy();

    /**
     * Main blocking application loop. Does message processing and handles rendering.
     * @return
     */
    int Run();

    // Accessor for the main window handle
    HWND GetHWND() const {
        return mHWnd;
    }

  private:
    // Internal method to handle user input.
    bool OnUserInput();

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
    static LRESULT CALLBACK OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // The instance-specific message handler where most of the window's message processing logic
    // resides.
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // --- Message Handlers ---
    // Handles the WM_CREATE message, typically used for creating child windows and initializing UI.
    bool OnCreate(HWND hWnd, LPCREATESTRUCT pcs);

    // Handles the WM_SIZE message, used for resizing and repositioning child windows.
    void OnSize(int clientWidth, int clientHeight);

  private:
    bool mIsRunning{true};
  
    HWND mHWnd;
    // Handles for the splitter bar windows
    HWND mHwndSplitter1;
    HWND mHwndSplitter2;

    // Instance handle for the application, obtained during creation.
    HINSTANCE mHInstance;

    // Smart pointers to manage the lifetime of our view components.
    std::unique_ptr<FileView> mFileView;
    std::unique_ptr<SceneTree> mSceneTree;
    std::unique_ptr<SceneView> mSceneView;
    std::unique_ptr<BaseFileProvider> mFileProvider;

    std::unique_ptr<Camera> mCamera;
    std::unique_ptr<GraphicsContext> mGraphicsContext;
};