// src/Window/SceneView.h
// Created by dtcimbal on 26/05/2025.
#pragma once

#include "BaseView.h"

#include <memory>

#include "Graphics/Device.h"

using std::unique_ptr;

// Define a unique class name for the SceneView window
const static LPCWSTR SCENE_VIEW_CLASS_NAME = L"DXMiniAppSceneView";

class SceneView : public BaseView {
  public:
    SceneView();
    ~SceneView() override;

    // Overrides BaseView::Create to create a custom window for the scene.
    bool OnCreate(HWND hParent, UINT id) override;

  private:
    unique_ptr<Device> mDevice;

    // --- Private helper methods for window management ---
    // Registers the window class for the SceneView window.
    ATOM RegisterWindowClass();

    // The static Window Procedure (trampoline) that dispatches messages to the correct SceneView
    // instance.
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // The instance-specific message handler where most of the window's message processing logic
    // resides.
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Message Handlers
    void OnSize(int displayWidth, int displayHeight);
};