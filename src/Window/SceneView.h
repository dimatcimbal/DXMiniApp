// src/Window/SceneView.h
// Created by dtcimbal on 26/05/2025.
#pragma once

#include "BaseView.h"

#include <Graphics/GraphicsContext.h>
#include <memory>

// Define a unique class name for the SceneView window
const static LPCWSTR SCENE_VIEW_CLASS_NAME = L"DXMiniAppSceneView";

class SceneView : public BaseView {
  public:
    SceneView();
    ~SceneView() override;

    // Overrides BaseView::Create to create a custom window for the scene.
    bool OnCreate(HWND hParent, UINT id) override;
    void OnDestroy() override;
    void OnResize(int Width, int Height) const;

    // Private methods
  private:
    // --- Private helper methods for window management ---
    // Registers the window class for the SceneView window.
    bool RegisterWindowClass();

    // The static Window Procedure (trampoline) that dispatches messages to the correct SceneView
    // instance.
    static LRESULT CALLBACK OnWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    // The instance-specific message handler where most of the window's message processing logic
    // resides.
    LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Private instance values
  private:
    ATOM mAtom{0};
};