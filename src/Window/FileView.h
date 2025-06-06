// src/Window/FileView.h
// Created by dtcimbal on 26/05/2025.
#pragma once

#include "BaseView.h"
#include "Util/BaseFileProvider.h"

class FileView : public BaseView {
  public:
    FileView(Util::BaseFileProvider& fileProvider);
    ~FileView() override;

    // Overrides BaseView::Create to create the ListView control.
    bool OnCreate(HWND hParent, UINT id) override;

    // Specific logic for populating the file list.
    void PopulateFileView();

  private:
    Util::BaseFileProvider& mFileProvider;
};