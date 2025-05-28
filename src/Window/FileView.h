#pragma once
#include <windows.h>
#include <CommCtrl.h>

namespace Window {
// Function to populate a ListBox with files from the current directory
void PopulateFileView(HWND hListBox);

// Function to handle file selection logic from TreeView
void HandleFileSelection(HWND hTreeView, LPNMTREEVIEW lpnmtv, HWND hSceneView, HWND hSceneTree);
} // namespace Window