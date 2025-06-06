// src/Window/SceneView.h
// Created by dtcimbal on 26/05/2025.
#pragma once

#include "BaseView.h"

#include <memory>

#include "Graphics/Device.h"

using std::unique_ptr;

class SceneView : public BaseView {
  public:
    SceneView();
    ~SceneView() override;

    // Overrides BaseView::Create to create a placeholder window for the scene.
    bool OnCreate(HWND hParent, UINT id) override;

  private:
    unique_ptr<Graphics::Device> mDevice;
};