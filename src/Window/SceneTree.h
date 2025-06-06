// src/Window/SceneTree.h
// Created by dtcimbal on 2/06/2025.
#pragma once

#include <string>
#include "BaseView.h"

class SceneTree : public BaseView {
  public:
    SceneTree();
    ~SceneTree() override;

    // Overrides BaseView::Create to create the TreeView control.
    bool OnCreate(HWND hParent, UINT id) override;

    // TODO: Add methods for managing the tree (e.g., AddNode(), RemoveNode(), GetSelectedNode())
    void AddDummyNodes();
};