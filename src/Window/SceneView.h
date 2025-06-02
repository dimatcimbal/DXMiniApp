// src/Window/SceneView.h
#pragma once

#include "BaseView.h"

class SceneView : public BaseView {
  public:
    SceneView();
    ~SceneView() override;

    // Overrides BaseView::Create to create a placeholder window for the scene.
    bool OnCreate(HWND hParent, UINT id) override;

    // TODO: Add rendering specific methods here (e.g., InitDirectX(), RenderFrame())
};