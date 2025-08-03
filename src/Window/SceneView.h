// src/Window/SceneView.h
// Created by dtcimbal on 26/05/2025.
#pragma once

#include "BaseView.h"

#include <memory>

class Camera;
class Device;
class Renderer;

// Define a unique class name for the SceneView window
const static LPCWSTR SCENE_VIEW_CLASS_NAME = L"DXMiniAppSceneView";

class SceneView : public BaseView {
  public:
    SceneView();
    ~SceneView() override;

    // Overrides BaseView::Create to create a custom window for the scene.
    bool OnCreate(HWND hParent, UINT id) override;
    void OnResize(int Width, int Height);
    void OnUpdate();

  private:
    std::unique_ptr<Device> mDevice;
    std::unique_ptr<Renderer> mRenderer;
    std::unique_ptr<Camera> mCamera;

    // --- Private helper methods for window management ---
    // Registers the window class for the SceneView window.
    ATOM RegisterWindowClass();

    // The static Window Procedure (trampoline) that dispatches messages to the correct SceneView
    // instance.
    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // The instance-specific message handler where most of the window's message processing logic
    // resides.
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};