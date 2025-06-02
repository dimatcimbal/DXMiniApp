// src/Window/SceneTree.h
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